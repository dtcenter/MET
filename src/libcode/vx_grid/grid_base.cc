// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"
#include "vx_log.h"
#include "grid_base.h"
#include "find_grid_by_name.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


static const int grid_debug_level = 4;


///////////////////////////////////////////////////////////////////////////////


inline bool is_even(int k) { return (k%2) == 0; }


////////////////////////////////////////////////////////////////////////


static int ll_func(double x_center, int N);


////////////////////////////////////////////////////////////////////////


   //
   //  Print grid data definitions
   //


////////////////////////////////////////////////////////////////////////


void LatLonData::dump() const

{

mlog << Debug(grid_debug_level)
     << "\nLatitude/Longitude Grid Data:\n"
     << "     lat_ll: " << lat_ll << "\n"
     << "     lon_ll: " << lon_ll << "\n"
     << "  delta_lat: " << delta_lat << "\n"
     << "  delta_lon: " << delta_lon << "\n"
     << "       Nlat: " << Nlat << "\n"
     << "       Nlon: " << Nlon << "\n\n";

}


////////////////////////////////////////////////////////////////////////


void RotatedLatLonData::dump() const

{

mlog << Debug(grid_debug_level)

     << "\nRotated Latitude/Longitude Grid Data:\n"

     << "            rot_lat_ll: " << rot_lat_ll << "\n"
     << "            rot_lon_ll: " << rot_lon_ll << "\n"

     << "         delta_rot_lat: " << delta_rot_lat << "\n"
     << "         delta_rot_lon: " << delta_rot_lon << "\n"

     << "                  Nlat: " << Nlat << "\n"
     << "                  Nlon: " << Nlon << "\n"

     << "   true_lat_south_pole: " << true_lat_south_pole << "\n"
     << "   true_lon_south_pole: " << true_lon_south_pole << "\n"

     << "          aux_rotation: " << aux_rotation << "\n\n";


return;

}


////////////////////////////////////////////////////////////////////////


void RotatedLatLonData::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "\nRotated Latitude/Longitude Grid Data:\n"

    << prefix << "            rot_lat_ll: " << rot_lat_ll << "\n"
    << prefix << "            rot_lon_ll: " << rot_lon_ll << "\n"

    << prefix << "         delta_rot_lat: " << delta_rot_lat << "\n"
    << prefix << "         delta_rot_lon: " << delta_rot_lon << "\n"

    << prefix << "                  Nlat: " << Nlat << "\n"
    << prefix << "                  Nlon: " << Nlon << "\n"

    << prefix << "   true_lat_south_pole: " << true_lat_south_pole << "\n"
    << prefix << "   true_lon_south_pole: " << true_lon_south_pole << "\n"

    << prefix << "          aux_rotation: " << aux_rotation << "\n\n";

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void MercatorData::dump() const

{

mlog << Debug(grid_debug_level)
     << "\nMercator Grid Data:\n"
     << "  lat_ll: " << lat_ll << "\n"
     << "  lon_ll: " << lon_ll << "\n"
     << "  lat_ur: " << lat_ur << "\n"
     << "  lon_ur: " << lon_ur << "\n"
     << "      nx: " << nx << "\n"
     << "      ny: " << ny << "\n\n";

}


////////////////////////////////////////////////////////////////////////


void LambertData::dump() const

{

mlog << Debug(grid_debug_level)
     << "\nLambert Conformal Grid Data:\n"
     << "   hemisphere: " << hemisphere << "\n"
     << "  scale_lat_1: " << scale_lat_1 << "\n"
     << "  scale_lat_2: " << scale_lat_2 << "\n"
     << "      lat_pin: " << lat_pin << "\n"
     << "      lon_pin: " << lon_pin << "\n"
     << "        x_pin: " << x_pin << "\n"
     << "        y_pin: " << y_pin << "\n"
     << "   lon_orient: " << lon_orient << "\n"
     << "         d_km: " << d_km << "\n"
     << "         r_km: " << r_km << "\n"
     << "           nx: " << nx << "\n"
     << "           ny: " << ny << "\n"
     << "    so2_angle: " << so2_angle << "\n\n";

}

////////////////////////////////////////////////////////////////////////


void StereographicData::dump() const

{

mlog << Debug(grid_debug_level)
     << "\nStereographic Grid Data:\n"
     << "  hemisphere: " << hemisphere << "\n"
     << "   scale_lat: " << scale_lat << "\n"
     << "     lat_pin: " << lat_pin << "\n"
     << "     lon_pin: " << lon_pin << "\n"
     << "       x_pin: " << x_pin << "\n"
     << "       y_pin: " << y_pin << "\n"
     << "  lon_orient: " << lon_orient << "\n"
     << "        d_km: " << d_km << "\n"
     << "       dy_km: " << dy_km << "\n"
     << "        r_km: " << r_km << "\n"
     << "          nx: " << nx << "\n"
     << "          ny: " << ny << "\n\n";

}


////////////////////////////////////////////////////////////////////////


void GaussianData::dump() const

{

mlog << Debug(grid_debug_level)
     << "\nGaussian Grid Data:\n"
     << "  lon_zero: " << lon_zero << "\n"
     << "        nx: " << nx << "\n"
     << "        ny: " << ny << "\n\n";

}


////////////////////////////////////////////////////////////////////////


void LaeaNetcdfData::dump() const

{

mlog << Debug(grid_debug_level)
     << "\nLaea NetCDF Grid Data:\n"
     << "                 name: " << name                      << "\n"
     << "   prime_meridian_lon: " << prime_meridian_lon        << "\n"
     << "   semi_major_axis_km: " << semi_major_axis_km        << "\n"
     << "   semi_minor_axis_km: " << semi_minor_axis_km        << "\n"
     << "      proj_origin_lat: " << proj_origin_lat           << "\n"
     << "      proj_origin_lon: " << proj_origin_lon           << "\n"
     << "                x_pin: " << x_pin                     << "\n"
     << "                y_pin: " << y_pin                     << "\n"
     << "                dx_km: " << dx_km                     << "\n"
     << "                dy_km: " << dy_km                     << "\n"
     << "                   nx: " << nx                        << "\n"
     << "                   ny: " << ny                        << "\n\n";

}


////////////////////////////////////////////////////////////////////////


void LaeaData::dump() const

{

mlog << Debug(grid_debug_level)
     << "\nLaea Grid Data:\n"
     << "        spheroid_name: " << spheroid_name             << "\n"
     << "            radius_km: " << radius_km                 << "\n"
     << " equatorial_radius_km: " << equatorial_radius_km      << "\n"
     << "      polar_radius_km: " << polar_radius_km           << "\n"
     << "            lat_first: " << lat_first                 << "\n"
     << "            lon_first: " << lon_first                 << "\n"
     << "         standard_lat: " << standard_lat              << "\n"
     << "          central_lon: " << central_lon               << "\n"
     << "                dx_km: " << dx_km                     << "\n"
     << "                dy_km: " << dy_km                     << "\n"
     << "                   nx: " << nx                        << "\n"
     << "                   ny: " << ny                        << "\n"
     << "            is_sphere: " << bool_to_string(is_sphere) << "\n\n";

}


////////////////////////////////////////////////////////////////////////


void SemiLatLonData::dump() const

{

mlog << Debug(grid_debug_level)
     << "\nSemiLatLon Grid Data:\n"
     << "    lats: " << lats.summarize() << "\n"
     << "    lons: " << lons.summarize() << "\n"
     << "  levels: " << levels.summarize() << "\n"
     << "   times: " << times.summarize() << "\n\n";
}


////////////////////////////////////////////////////////////////////////


void SemiLatLonData::clear()

{

name = (const char *) nullptr;

lats.clear();
lons.clear();
levels.clear();
times.clear();

}


////////////////////////////////////////////////////////////////////////

#ifdef WITH_UGRID

void UnstructuredData::clear() {

   name = (const char *) nullptr;

   max_distance_km = bad_data_double;  // disable max_distance

   clear_data();

}


////////////////////////////////////////////////////////////////////////

void UnstructuredData::clear_data() {

   n_face = n_node = n_edge = 0;
   point_lonlat.clear();
   lat_checksum = lon_checksum = 0.;

   if (kdtree) { delete kdtree; kdtree = nullptr; }

}


////////////////////////////////////////////////////////////////////////


void UnstructuredData::dump() const {

   mlog << Debug(grid_debug_level)
        << "\nUnstructured Grid Data:\n"
        << "           n_face: " << n_face << "\n"
        << "    lat_checksum: " << lat_checksum << "\n"
        << "    lon_checksum: " << lon_checksum << "\n"
        << " max_distance_km: " << max_distance_km << "\n"
        ;
}

#endif


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GridInfo
   //


////////////////////////////////////////////////////////////////////////


GridInfo::GridInfo()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


GridInfo::~GridInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


GridInfo::GridInfo(const GridInfo & info)

{

init_from_scratch();

assign(info);

}


////////////////////////////////////////////////////////////////////////


GridInfo & GridInfo::operator=(const GridInfo & info)

{

if ( this != &info ) assign(info);

return *this;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::init_from_scratch()

{

lc  = (const LambertData *)       nullptr;
st  = (const StereographicData *) nullptr;
ll  = (const LatLonData *)        nullptr;
rll = (const RotatedLatLonData *) nullptr;
m   = (const MercatorData *)      nullptr;
g   = (const GaussianData *)      nullptr;
gi  = (const GoesImagerData *)    nullptr;
la  = (const LaeaData *)          nullptr;
tc  = (const TcrmwData *)         nullptr;
sl  = (const SemiLatLonData *)    nullptr;
#ifdef WITH_UGRID
us  = (const UnstructuredData *)  nullptr;
#endif

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::clear()

{

if ( lc  )  { delete lc;   lc  = (const LambertData *)       nullptr; };
if ( st  )  { delete st;   st  = (const StereographicData *) nullptr; };
if ( ll  )  { delete ll;   ll  = (const LatLonData *)        nullptr; };
if ( rll )  { delete rll;  rll = (const RotatedLatLonData *) nullptr; };
if ( m   )  { delete m;    m   = (const MercatorData *)      nullptr; };
if ( g   )  { delete g;    g   = (const GaussianData *)      nullptr; };
if ( gi  )  { delete gi;   gi  = (const GoesImagerData *)    nullptr; };
if ( la  )  { delete la;   la  = (const LaeaData *)          nullptr; };
if ( tc  )  { delete tc;   tc  = (const TcrmwData *)         nullptr; };
if ( sl  )  { delete sl;   sl  = (const SemiLatLonData *)    nullptr; };
#ifdef WITH_UGRID
if ( us  )  { delete us;   us  = (const UnstructuredData *)  nullptr; };
#endif

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::assign(const GridInfo & info)

{

if ( info.lc  )  set( *(info.lc)  );
if ( info.st  )  set( *(info.st)  );
if ( info.ll  )  set( *(info.ll)  );
if ( info.rll )  set( *(info.rll) );
if ( info.m   )  set( *(info.m)   );
if ( info.g   )  set( *(info.g)   );
if ( info.gi  )  set( *(info.gi)  );
if ( info.la  )  set( *(info.la)  );
if ( info.sl  )  set( *(info.sl)  );
#ifdef WITH_UGRID
if ( info.us  )  set( *(info.us)  );
#endif

return;

}


////////////////////////////////////////////////////////////////////////


bool GridInfo::ok() const

{

int count = 0;

if ( lc  ) ++count;
if ( st  ) ++count;
if ( ll  ) ++count;
if ( rll ) ++count;
if ( m   ) ++count;
if ( g   ) ++count;
if ( gi  ) ++count;
if ( la  ) ++count;
if ( sl  ) ++count;
#ifdef WITH_UGRID
if ( us  ) ++count;
#endif

return count == 1;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::create_grid(Grid & gg) const

{

if ( !(ok()) )  {

   mlog << Error << "\nGridInfo::create_grid(Grid &) const -> bad gridinfo\n\n";

   exit ( 1 );

}

     if ( lc  )  gg.set( *lc  );
else if ( st  )  gg.set( *st  );
else if ( ll  )  gg.set( *ll  );
else if ( rll )  gg.set( *rll );
else if ( m   )  gg.set( *m   );
else if ( g   )  gg.set( *g   );
else if ( gi  )  gg.set( *gi  );
else if ( la  )  gg.set( *la  );
else if ( sl  )  gg.set( *sl  );
#ifdef WITH_UGRID
else if ( us  )  gg.set( *us  );
#endif

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const LambertData & data)

{

clear();

LambertData * D = (LambertData *) nullptr;

D = new LambertData;

memcpy(D, &data, sizeof(data));

lc = D;  D = (LambertData *) nullptr;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const StereographicData & data)

{

clear();

StereographicData * D = (StereographicData *) nullptr;

D = new StereographicData;

memcpy(D, &data, sizeof(data));

st = D;  D = (StereographicData *) nullptr;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const LatLonData & data)

{

clear();

LatLonData * D = (LatLonData *) nullptr;

D = new LatLonData;

memcpy(D, &data, sizeof(data));

ll = D;  D = (LatLonData *) nullptr;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const RotatedLatLonData & data)

{

clear();

RotatedLatLonData * D = (RotatedLatLonData *) nullptr;

D = new RotatedLatLonData;

memcpy(D, &data, sizeof(data));

rll = D;  D = (RotatedLatLonData *) nullptr;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const MercatorData & data)

{

clear();

MercatorData * D = (MercatorData *) nullptr;

D = new MercatorData;

memcpy(D, &data, sizeof(data));

m = D;  D = (MercatorData *) nullptr;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const GaussianData & data)

{

clear();

GaussianData * D = (GaussianData *) nullptr;

D = new GaussianData;

memcpy(D, &data, sizeof(data));

g = D;  D = (GaussianData *) nullptr;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const GoesImagerData & data)

{

clear();

GoesImagerData * D = (GoesImagerData *) nullptr;

D = new GoesImagerData;

memcpy(D, &data, sizeof(data));

gi = D;  D = (GoesImagerData *) nullptr;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const LaeaData & data)

{

clear();

LaeaData * D = (LaeaData *) nullptr;

D = new LaeaData;

memcpy(D, &data, sizeof(data));

la = D;  D = (LaeaData *) nullptr;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const SemiLatLonData & data)

{

clear();

SemiLatLonData * D = (SemiLatLonData *) nullptr;

D = new SemiLatLonData;

   //
   //  deep copy instead of memcpy because the struct
   //  contains dynamically allocated NumArray objects
   //

*D = data;

sl = D;  D = (SemiLatLonData *) nullptr;

return;

}


////////////////////////////////////////////////////////////////////////


#ifdef WITH_UGRID
void GridInfo::set(const UnstructuredData & data)

{

clear();

UnstructuredData *D = new UnstructuredData;

D->n_edge = data.n_edge;
D->n_node = data.n_node;
D->max_distance_km = data.max_distance_km;
D->set_points(data.n_face, data.point_lonlat);
us = D;
D = (UnstructuredData *)nullptr;

}
#endif


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GridInterface
   //


////////////////////////////////////////////////////////////////////////


GridInterface::GridInterface()

{

}


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

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Grid::~Grid()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Grid::Grid(const Grid & g)

{

init_from_scratch();

assign(g);

}


////////////////////////////////////////////////////////////////////////


Grid & Grid::operator=(const Grid & g)

{

if ( this != &g ) assign(g);

return *this;

}


////////////////////////////////////////////////////////////////////////


Grid::Grid(const char * _name)

{

init_from_scratch();

set(_name);

}


////////////////////////////////////////////////////////////////////////


void Grid::init_from_scratch()

{

rep = (GridRep *) nullptr;

clear();

}


////////////////////////////////////////////////////////////////////////


void Grid::clear()

{

if ( rep )  { delete rep;  rep = (GridRep *) nullptr; }
set_swap_to_north(false);

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::assign(const Grid & g)

{

clear();

if ( ! (g.rep) )  return;

rep = g.rep->copy();
set_swap_to_north(g.get_swap_to_north());

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const char * _name)

{

clear();

bool status = find_grid_by_name(_name, *this);

if ( !status )  {

   mlog << Error << "\nGrid::set(const char *) -> "
        << "grid lookup failed for name \""
        << _name << "\"\n\n";

   exit ( 1 );

}


return;

}


////////////////////////////////////////////////////////////////////////

bool Grid::get_swap_to_north() const {
   return swap_to_north && (info().ll != nullptr || info().rll != nullptr);
}

////////////////////////////////////////////////////////////////////////

void Grid::set_swap_to_north(bool cur_swap_to_north) {
   this->swap_to_north = cur_swap_to_north;
}

////////////////////////////////////////////////////////////////////////


void Grid::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Grid Base ...\n";

if ( rep )  rep->dump(out, depth + 1);

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

if ( !rep )  return 0.0;

return rep->calc_area(x, y);

}


////////////////////////////////////////////////////////////////////////


int Grid::nx() const

{

if ( !rep )  return 0;

return rep->nx();

}


////////////////////////////////////////////////////////////////////////


int Grid::ny() const

{

if ( !rep )  return 0;

return rep->ny();

}


////////////////////////////////////////////////////////////////////////


int Grid::nxy() const

{

return nx()*ny();

}


////////////////////////////////////////////////////////////////////////


double Grid::scale_km() const

{

if ( !rep )  return 0;

return rep->scale_km();

}


////////////////////////////////////////////////////////////////////////


ConcatString Grid::name() const

{

if ( !rep )  return ConcatString("(no name)");

return rep->name();

}


////////////////////////////////////////////////////////////////////////


ConcatString Grid::serialize(const char *sep) const

{

ConcatString s;

if ( rep )  s = rep->serialize(sep);

return s;

}


////////////////////////////////////////////////////////////////////////


GridInfo Grid::info() const

{

if ( !rep )  {

   mlog << Error << "\nGrid::info() const -> "
        << "empty grid!\n\n";

   exit ( 1 );

}

return rep->info();

}


////////////////////////////////////////////////////////////////////////


double Grid::rot_grid_to_earth(int x, int y) const

{

if ( !rep )  {

   mlog << Error << "\nGrid::rot_grid_to_earth() const -> "
        << "empty grid!\n\n";

   exit ( 1 );

}


return rep->rot_grid_to_earth(x, y);

}


////////////////////////////////////////////////////////////////////////


bool Grid::wrap_lon() const

{

if ( !rep )  {

   mlog << Error << "\nGrid::wrap_lon() const -> "
        << "empty grid!\n\n";

   exit ( 1 );

}

return rep->wrap_lon();

}


////////////////////////////////////////////////////////////////////////


void Grid::shift_right(int N)

{

if ( !rep )  {

   mlog << Error << "\nGrid::shift_right() -> "
        << "empty grid!\n\n";

   exit ( 1 );

}

return rep->shift_right(N);

}


////////////////////////////////////////////////////////////////////////


Grid Grid::subset_ll(int x_ll, int y_ll, int nx_new, int ny_new) const

{

if ( ! rep )  {

   mlog << Error << "\n\n  Grid::subset_ll() const -> "
        << "empty grid!\n\n";

   exit ( 1 );

}

if ( (nx_new < 2) || ( ny_new < 2) )  {

   mlog << Error << "\n\n  Grid::subset_ll() const -> "
        << "bad size for subset grid\n\n";

   exit ( 1 );

}

Grid g_new;
double lat_ll, lon_ll;
GridInfo info_new = info();


xy_to_latlon(x_ll, y_ll, lat_ll, lon_ll);


if ( info_new.lc )  {

   LambertData lc_new = *(info_new.lc);

   lc_new.lat_pin = lat_ll;
   lc_new.lon_pin = lon_ll;

   lc_new.x_pin = 0.0;
   lc_new.y_pin = 0.0;

   lc_new.nx = nx_new;
   lc_new.ny = ny_new;

   g_new.set(lc_new);

} else if ( info_new.st )  {

   StereographicData st_new = *(info_new.st);

   st_new.lat_pin = lat_ll;
   st_new.lon_pin = lon_ll;

   st_new.x_pin = 0.0;
   st_new.y_pin = 0.0;

   st_new.nx = nx_new;
   st_new.ny = ny_new;

   g_new.set(st_new);

} else if ( info_new.ll )  {

   LatLonData ll_new = *(info_new.ll);

   ll_new.lat_ll = lat_ll;
   ll_new.lon_ll = lon_ll;

   ll_new.Nlat = ny_new;
   ll_new.Nlon = nx_new;

   g_new.set(ll_new);

} else if ( info_new.m )  {

   MercatorData m_new = *(info_new.m);
   double lat_ur, lon_ur;

   xy_to_latlon(x_ll + nx_new - 1, y_ll + ny_new - 1, lat_ur, lon_ur);

   m_new.lat_ll = lat_ll;
   m_new.lon_ll = lon_ll;

   m_new.lat_ur = lat_ur;
   m_new.lon_ur = lon_ur;

   g_new.set(m_new);

} else if ( info_new.sl )  {

   mlog << Error << "\n\n  Grid::subset_ll() const -> "
        << "subsetting is not supported for SemiLatLon grids\n\n";

   exit ( 1 );

} else {

   mlog << Error << "\n\n  Grid::subset_ll() const -> "
        << "bad grid projection\n\n";

   exit ( 1 );

}

return g_new;

}


////////////////////////////////////////////////////////////////////////


Grid Grid::subset_center(double lat_center, double lon_center, int nx_new, int ny_new) const

{

   //
   //  subset_ll does sanity checking on nx_new and ny_new, so we don't have to do it here
   //

int ix_ll, iy_ll;
double dx_center, dy_center;


   //
   //  find the (floating-point) grid coords corresponding
   //
   //     to the given point (lat_center, lon_center)
   //

latlon_to_xy(lat_center, lon_center, dx_center, dy_center);

   //
   //  figure out (old grid) coordinates of the new grid's lower-left corner
   //

ix_ll = ll_func(dx_center, nx_new);
iy_ll = ll_func(dy_center, ny_new);

   //
   //  subset
   //

return subset_ll(ix_ll, iy_ll, nx_new, ny_new);

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


bool operator==(const Grid & g1, const Grid & g2)

{

if ( !(g1.rep) ) return false;
if ( !(g2.rep) ) return false;

return g1.info() == g2.info();

}


////////////////////////////////////////////////////////////////////////


bool operator!=(const Grid & g1, const Grid & g2)

{

bool status = (g1 == g2);

return ! status;

}


////////////////////////////////////////////////////////////////////////


bool operator==(const GridInfo & i1, const GridInfo & i2)

{

     if ( i1.lc  && i2.lc  )  return ( is_eq(i1.lc,  i2.lc ) );
else if ( i1.st  && i2.st  )  return ( is_eq(i1.st,  i2.st ) );
else if ( i1.ll  && i2.ll  )  return ( is_eq(i1.ll,  i2.ll ) );
else if ( i1.rll && i2.rll )  return ( is_eq(i1.rll, i2.rll) );
else if ( i1.m   && i2.m   )  return ( is_eq(i1.m,   i2.m  ) );
else if ( i1.g   && i2.g   )  return ( is_eq(i1.g,   i2.g  ) );
else if ( i1.gi  && i2.gi  )  return ( is_eq(i1.gi,  i2.gi ) );
else if ( i1.la  && i2.la  )  return ( is_eq(i1.la,  i2.la ) );
else if ( i1.sl  && i2.sl  )  return ( is_eq(i1.sl,  i2.sl ) );
#ifdef WITH_UGRID
else if ( i1.us  && i2.us  )  return ( is_eq(i1.us,  i2.us ) );
#endif

return false;

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const LambertData * lc1, const LambertData * lc2)

{

if ( !lc1 || !lc2 )  return false;

bool status = false;

if ( lc1->nx                == lc2->nx                      &&
     lc1->ny                == lc2->ny                      &&
     is_eq  (lc1->scale_lat_1, lc2->scale_lat_1, loose_tol) &&
     is_eq  (lc1->scale_lat_2, lc2->scale_lat_2, loose_tol) &&
     is_eq  (lc1->lat_pin,     lc2->lat_pin,     loose_tol) &&
     is_eq  (rescale_lon(lc1->lon_pin),
             rescale_lon(lc2->lon_pin),          loose_tol) &&
     is_eq  (lc1->x_pin,       lc2->x_pin,       loose_tol) &&
     is_eq  (lc1->y_pin,       lc2->y_pin,       loose_tol) &&
     is_eq  (rescale_lon(lc1->lon_orient),
             rescale_lon(lc2->lon_orient),       loose_tol) &&
     is_eq  (lc1->d_km,        lc2->d_km,        loose_tol) &&
     is_eq  (lc1->r_km,        lc2->r_km,        loose_tol) &&
     is_eq  (lc1->so2_angle,   lc2->so2_angle,   loose_tol) )  status = true;

return status;

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const StereographicData * st1, const StereographicData * st2)

{

if ( !st1 || !st2 )  return false;

bool status = false;

if ( st1->nx               == st2->nx                    &&
     st1->ny               == st2->ny                    &&
     st1->hemisphere       == st2->hemisphere            &&
     is_eq  (st1->scale_lat,  st2->scale_lat, loose_tol) &&
     is_eq  (st1->lat_pin,    st2->lat_pin,   loose_tol) &&
     is_eq  (rescale_lon(st1->lon_pin),
             rescale_lon(st2->lon_pin),       loose_tol) &&
     is_eq  (st1->x_pin,      st2->x_pin,     loose_tol) &&
     is_eq  (st1->y_pin,      st2->y_pin,     loose_tol) &&
     is_eq  (rescale_lon(st1->lon_orient),
             rescale_lon(st2->lon_orient),    loose_tol) &&
     is_eq  (st1->d_km,       st2->d_km,      loose_tol) &&
     is_eq  (st1->dy_km,      st2->dy_km,     loose_tol) &&
     is_eq  (st1->r_km,       st2->r_km,      loose_tol) )  status = true;

return status;

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const LatLonData * ll1, const LatLonData * ll2)

{

if ( !ll1 || !ll2 )  return false;

bool status = false;

if ( ll1->Nlat            == ll2->Nlat                  &&
     ll1->Nlon            == ll2->Nlon                  &&
     is_eq  (ll1->lat_ll,    ll2->lat_ll,    loose_tol) &&
     is_eq  (rescale_lon(ll1->lon_ll),
             rescale_lon(ll2->lon_ll),       loose_tol) &&
     is_eq  (ll1->delta_lat, ll2->delta_lat, loose_tol) &&
     is_eq  (ll1->delta_lon, ll2->delta_lon, loose_tol) )  status = true;

return status;

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const RotatedLatLonData * ll1, const RotatedLatLonData * ll2)

{

if ( !ll1 || !ll2 ) return false;

bool status = false;

if ( ll1->Nlat == ll2->Nlat &&
     ll1->Nlon == ll2->Nlon &&
     is_eq  (ll1->rot_lat_ll,
             ll2->rot_lat_ll, loose_tol) &&
     is_eq  (rescale_lon(ll1->rot_lon_ll),
             rescale_lon(ll2->rot_lon_ll), loose_tol) &&
     is_eq  (ll1->delta_rot_lat,
             ll2->delta_rot_lat, loose_tol) &&
     is_eq  (ll1->delta_rot_lon,
             ll2->delta_rot_lon, loose_tol) &&
     is_eq  (ll1->true_lat_south_pole,
             ll2->true_lat_south_pole, loose_tol) &&
     is_eq  (rescale_lon(ll1->true_lon_south_pole),
             rescale_lon(ll2->true_lon_south_pole), loose_tol) &&
     is_eq  (ll1->aux_rotation,
             ll2->aux_rotation, loose_tol) )  status = true;

return status;

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const MercatorData * m1, const MercatorData * m2)

{

if ( !m1 || !m2 ) return false;

bool status = false;

if ( m1->nx           == m2->nx                  &&
     m1->ny           == m2->ny                  &&
     is_eq  (m1->lat_ll, m2->lat_ll,  loose_tol) &&
     is_eq  (rescale_lon(m1->lon_ll),
             rescale_lon(m2->lon_ll), loose_tol) &&
     is_eq  (m1->lat_ur, m2->lat_ur,  loose_tol) &&
     is_eq  (rescale_lon(m1->lon_ur),
             rescale_lon(m2->lon_ur), loose_tol) )  status = true;

return status;

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const GaussianData * g1, const GaussianData * g2)

{

if ( !g1 || !g2 ) return false;

bool status = false;

if ( is_eq  (rescale_lon(g1->lon_zero),
             rescale_lon(g2->lon_zero), loose_tol) &&
     g1->nx  == g2->nx                             &&
     g1->ny  == g2->ny )  status = true;

return status;

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const GoesImagerData * gi1, const GoesImagerData * gi2)
{

if ( !gi1 || !gi2 ) return false;

bool status = false;

if ( gi1->nx           == gi2->nx             &&
     gi1->ny           == gi2->ny             &&
     is_eq  (gi1->semi_major_axis,
             gi2->semi_major_axis, loose_tol) &&
     is_eq  (gi1->semi_minor_axis,
             gi2->semi_minor_axis, loose_tol) &&
     is_eq  (gi1->inverse_flattening,
             gi2->inverse_flattening, loose_tol) &&
     is_eq  (gi1->perspective_point_height,
             gi2->perspective_point_height, loose_tol) &&
     is_eq  (gi1->lat_of_projection_origin,
             gi2->lat_of_projection_origin, loose_tol) &&
     is_eq  (gi1->lon_of_projection_origin,
             gi2->lon_of_projection_origin, loose_tol) &&
     is_eq  (gi1->dx_rad, gi2->dx_rad, loose_tol) &&
     is_eq  (gi1->dy_rad, gi2->dy_rad, loose_tol) )  status = true;

return status;

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const LaeaData * g1, const LaeaData * g2)

{

if ( !g1 || !g2 ) return false;

bool status = false;

if ( g1->radius_km            == g2->radius_km &&
     g1->equatorial_radius_km == g2->equatorial_radius_km &&
     g1->polar_radius_km      == g2->polar_radius_km &&
     g1->lat_first            == g2->lat_first &&
     is_eq (rescale_lon(g1->lon_first),
            rescale_lon(g2->lon_first), loose_tol) &&
     g1->standard_lat         == g2->standard_lat &&
     is_eq (rescale_lon(g1->central_lon),
            rescale_lon(g2->central_lon), loose_tol) &&
     g1->dx_km                == g2->dx_km &&
     g1->dy_km                == g2->dy_km &&
     g1->nx                   == g2->nx &&
     g1->ny                   == g2->ny &&
     g1->is_sphere            == g2->is_sphere )  status = true;

return status;

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const SemiLatLonData * gi1, const SemiLatLonData * gi2)

{

if ( !gi1 || !gi2 ) return false;

bool status = false;

if ( gi1->lats   == gi2->lats   &&
     gi1->lons   == gi2->lons   &&
     gi1->levels == gi2->levels &&
     gi1->times  == gi2->times )  status = true;


return status;

}


////////////////////////////////////////////////////////////////////////

#ifdef WITH_UGRID
bool is_eq(const UnstructuredData * us1, const UnstructuredData * us2)

{

  bool status = false;
  if (us1 && us2) {
    if (us1 == us2) status = true;
    else status = us1->n_face == us2->n_face
                  && us1->n_node == us2->n_node
                  && us1->n_edge == us2->n_edge
                  && us1->point_lonlat[0] == us2->point_lonlat[0]
                  && (us1->n_face > 0 && us1->point_lonlat[us1->n_face-1] == us2->point_lonlat[us2->n_face-1])
                  && is_eq(us1->lat_checksum, us2->lat_checksum)
                  && is_eq(us1->lon_checksum, us2->lon_checksum);
  }

return status;

}
#endif


////////////////////////////////////////////////////////////////////////


int ll_func(double x_center, int N)

{

int LL;

if ( is_even(N) )  {

   LL = nint(floor(x_center)) - ((N/2) - 1);

} else {

   LL = nint(x_center) - ((N - 1)/2);

}


return LL;

}


////////////////////////////////////////////////////////////////////////
