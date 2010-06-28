

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "misc.h"
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
e  = (const ExpData *)           0;

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
if ( e  )  { delete e;  e  = (const ExpData *)           0; };

return;

}


////////////////////////////////////////////////////////////////////////


void GridInfo::assign(const GridInfo & info)

{

if ( info.lc )  set( *(info.lc) );
if ( info.st )  set( *(info.st) );
if ( info.ll )  set( *(info.ll) );
if ( info.m  )  set( *(info.m ) );
if ( info.e  )  set( *(info.e ) );

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
if ( e  ) ++count;

return ( count == 1 );

}


////////////////////////////////////////////////////////////////////////


void GridInfo::create_grid(Grid & g) const 

{

if ( !(ok()) )  {

   cerr << "\n\n  GridInfo::create_grid(Grid &) const -> bad gridinfo\n\n";

   exit ( 1 );

}

     if ( lc )  g.set( *lc );
else if ( st )  g.set( *st );
else if ( ll )  g.set( *ll );
else if ( m  )  g.set( *m  );
else if ( e  )  g.set( *e  );

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


void GridInfo::set(const ExpData & data)

{

clear();

ExpData * D = (ExpData *) 0;

D = new ExpData;

memcpy(D, &data, sizeof(data));

e = D;  D = (ExpData *) 0;

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

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Grid::~Grid()

{

detach();

}


////////////////////////////////////////////////////////////////////////


Grid::Grid(const Grid & g)

{

init_from_scratch();

assign(g);

// attach(g.rep);

}


////////////////////////////////////////////////////////////////////////


Grid & Grid::operator=(const Grid & g)

{

if ( this == &g )  return ( * this );

// attach(g.rep);

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

detach();

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::assign(const Grid & g)

{

clear();

attach(g.rep);

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::set(const char * _name)

{

clear();

bool status = find_grid_by_name(_name, *this);

if ( !status )  {

   cerr << "\n\n  Grid::set(const char *) -> grid lookup failed for name \"" << _name << "\"\n\n";

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


void Grid::detach()

{

if ( !rep )  return;

if ( --(rep->refCount) <= 0 )  { delete rep;  rep = (GridRep *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void Grid::attach(GridRep * r)

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

   cerr << "\n\n  Grid::info() const -> empty grid!\n\n";

   exit ( 1 );

}

return ( rep->info() );

}


////////////////////////////////////////////////////////////////////////


double Grid::rot_grid_to_earth(int x, int y) const

{

if ( !rep )  {

   cerr << "\n\n  Grid::rot_grid_to_earth() const -> empty grid!\n\n";

   exit ( 1 );

}


return ( rep->rot_grid_to_earth(x, y) );

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


ConcatString s1, s2;

s1 = g1.rep->serialize();
s1 = g2.rep->serialize();

return ( s1 == s2 );

}


////////////////////////////////////////////////////////////////////////




