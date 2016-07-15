

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"
#include "vx_log.h"
#include "grid_base.h"
#include "find_grid_by_name.h"


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

if ( this == &info )  return ( * this );

assign(info);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void GridInfo::init_from_scratch()

{

lc = (const LambertData *)       0;
st = (const StereographicData *) 0;
ll = (const LatLonData *)        0;
m  = (const MercatorData *)      0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::clear()

{

if ( lc )  { delete lc; lc = (const LambertData *)       0; };
if ( st )  { delete st; st = (const StereographicData *) 0; };
if ( ll )  { delete ll; ll = (const LatLonData *)        0; };
if ( m  )  { delete m;  m  = (const MercatorData *)      0; };

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::assign(const GridInfo & info)

{

if ( info.lc )  set( *(info.lc) );
if ( info.st )  set( *(info.st) );
if ( info.ll )  set( *(info.ll) );
if ( info.m  )  set( *(info.m ) );

return;

}


////////////////////////////////////////////////////////////////////////


bool GridInfo::ok() const

{

int count = 0;

if ( lc ) ++count;
if ( st ) ++count;
if ( ll ) ++count;
if ( m  ) ++count;

return ( count == 1 );

}


////////////////////////////////////////////////////////////////////////


void GridInfo::create_grid(Grid & g) const 

{

if ( !(ok()) )  {

   mlog << Error << "\nGridInfo::create_grid(Grid &) const -> bad gridinfo\n\n";

   exit ( 1 );

}

     if ( lc )  g.set( *lc );
else if ( st )  g.set( *st );
else if ( ll )  g.set( *ll );
else if ( m  )  g.set( *m  );

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const LambertData & data)

{

clear();

LambertData * D = (LambertData *) 0;

D = new LambertData;

memcpy(D, &data, sizeof(data));

lc = D;  D = (LambertData *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const StereographicData & data)

{

clear();

StereographicData * D = (StereographicData *) 0;

D = new StereographicData;

memcpy(D, &data, sizeof(data));

st = D;  D = (StereographicData *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const LatLonData & data)

{

clear();

LatLonData * D = (LatLonData *) 0;

D = new LatLonData;

memcpy(D, &data, sizeof(data));

ll = D;  D = (LatLonData *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::set(const MercatorData & data)

{

clear();

MercatorData * D = (MercatorData *) 0;

D = new MercatorData;

memcpy(D, &data, sizeof(data));

m = D;  D = (MercatorData *) 0;

return;

}


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

if ( this == &g )  return ( * this );

assign(g);

return ( * this );

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

rep = (GridRep *) 0;

clear();

}


////////////////////////////////////////////////////////////////////////


void Grid::clear()

{

if ( rep )  { delete rep;  rep = (GridRep *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::assign(const Grid & g)

{

clear();

if ( ! (g.rep) )  return;

rep = g.rep->copy();

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const char * _name)

{

clear();

bool status = find_grid_by_name(_name, *this);

if ( !status )  {

   mlog << Error << "\nGrid::set(const char *) -> grid lookup failed for name \"" << _name << "\"\n\n";

   exit ( 1 );

}


return;

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

if ( !rep )  return ( 0.0 );

return ( rep->calc_area(x, y) );

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


ConcatString Grid::name() const

{

if ( !rep )  return ( ConcatString("(no name)") );

return ( rep->name() );

}


////////////////////////////////////////////////////////////////////////


ConcatString Grid::serialize() const

{

ConcatString s;

if ( rep )  s = rep->serialize();

return ( s );

}


////////////////////////////////////////////////////////////////////////


GridInfo Grid::info() const

{

if ( !rep )  {

   mlog << Error << "\nGrid::info() const -> empty grid!\n\n";

   exit ( 1 );

}

return ( rep->info() );

}


////////////////////////////////////////////////////////////////////////


double Grid::rot_grid_to_earth(int x, int y) const

{

if ( !rep )  {

   mlog << Error << "\nGrid::rot_grid_to_earth() const -> empty grid!\n\n";

   exit ( 1 );

}


return ( rep->rot_grid_to_earth(x, y) );

}


////////////////////////////////////////////////////////////////////////


bool Grid::is_global() const

{

if ( !rep )  {

   mlog << Error << "\nGrid::is_global() const -> empty grid!\n\n";

   exit ( 1 );

}

return ( rep->is_global() );

}


////////////////////////////////////////////////////////////////////////


void Grid::shift_right(int N)

{

if ( !rep )  {

   mlog << Error << "\nGrid::shift_right() -> empty grid!\n\n";

   exit ( 1 );

}

return ( rep->shift_right(N) );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


bool operator==(const Grid & g1, const Grid & g2)

{

if ( !(g1.rep) )  return ( false );
if ( !(g2.rep) )  return ( false );

return ( g1.info() == g2.info() );

}


////////////////////////////////////////////////////////////////////////


bool operator!=(const Grid & g1, const Grid & g2)

{

bool status = (g1 == g2);

return ( ! status );

}


////////////////////////////////////////////////////////////////////////


bool operator==(const GridInfo & i1, const GridInfo & i2)

{

     if ( i1.lc && i2.lc )  return ( is_eq(i1.lc, i2.lc) );
else if ( i1.st && i2.st )  return ( is_eq(i1.st, i2.st) );
else if ( i1.ll && i2.ll )  return ( is_eq(i1.ll, i2.ll) );
else if ( i1.m  && i2.m  )  return ( is_eq(i1.m,  i2.m ) );

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const LambertData * lc1, const LambertData * lc2)

{

if ( !lc1 || !lc2 )  return ( false );

bool status = false;

if ( lc1->nx                == lc2->nx           &&
     lc1->ny                == lc2->ny           &&
     is_eq  (lc1->scale_lat_1, lc2->scale_lat_1) &&
     is_eq  (lc1->scale_lat_2, lc2->scale_lat_2) &&
     is_eq  (lc1->lat_pin,     lc2->lat_pin)     &&
     is_eq  (rescale_lon(lc1->lon_pin),
             rescale_lon(lc2->lon_pin))          &&
     is_eq  (lc1->x_pin,       lc2->x_pin)       &&
     is_eq  (lc1->y_pin,       lc2->y_pin)       &&
     is_eq  (rescale_lon(lc1->lon_orient),
             rescale_lon(lc2->lon_orient))       &&
     is_eq  (lc1->d_km,        lc2->d_km)        &&
     is_eq  (lc1->r_km,        lc2->r_km) )  status = true;

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const StereographicData * st1, const StereographicData * st2)

{
   
if ( !st1 || !st2 )  return ( false );

bool status = false;

if ( st1->nx               == st2->nx          &&
     st1->ny               == st2->ny          &&
     st1->hemisphere       == st2->hemisphere  &&
     is_eq  (st1->scale_lat,  st2->scale_lat)  &&
     is_eq  (st1->lat_pin,    st2->lat_pin)    &&
     is_eq  (rescale_lon(st1->lon_pin),
             rescale_lon(st2->lon_pin))        &&
     is_eq  (st1->x_pin,      st2->x_pin)      &&
     is_eq  (st1->y_pin,      st2->y_pin)      &&
     is_eq  (rescale_lon(st1->lon_orient),
             rescale_lon(st2->lon_orient))     &&
     is_eq  (st1->d_km,       st2->d_km)       &&
     is_eq  (st1->r_km,       st2->r_km) )  status = true;

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const LatLonData * ll1, const LatLonData * ll2)

{

if ( !ll1 || !ll2 )  return ( false );

bool status = false;

if ( ll1->Nlat            == ll2->Nlat       &&
     ll1->Nlon            == ll2->Nlon       &&
     is_eq  (ll1->lat_ll,    ll2->lat_ll)    &&
     is_eq  (rescale_lon(ll1->lon_ll),
             rescale_lon(ll2->lon_ll))       &&
     is_eq  (ll1->delta_lat, ll2->delta_lat) &&
     is_eq  (ll1->delta_lon, ll2->delta_lon) )  status = true;

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool is_eq(const MercatorData * m1, const MercatorData * m2)

{

if ( !m1 || !m2 )  return ( false );

bool status = false;

if ( m1->nx           == m2->nx       &&
     m1->ny           == m2->ny       &&
     is_eq  (m1->lat_ll, m2->lat_ll)  &&
     is_eq  (rescale_lon(m1->lon_ll),
             rescale_lon(m2->lon_ll)) &&
     is_eq  (m1->lat_ur, m2->lat_ur)  &&
     is_eq  (rescale_lon(m1->lon_ur),
             rescale_lon(m2->lon_ur)) )  status = true;

return ( status );

}


////////////////////////////////////////////////////////////////////////
