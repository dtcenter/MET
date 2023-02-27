// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

#include "find_grid_by_name.h"


////////////////////////////////////////////////////////////////////////


static bool parse_lambert_grid(const StringArray &, Grid &);

static bool parse_latlon_grid(const StringArray &, Grid &);

static bool parse_rotlatlon_grid(const StringArray &, Grid &);

static bool parse_stereographic_grid(const StringArray &, Grid &);

static bool parse_mercator_grid(const StringArray &, Grid &);

static bool parse_gaussian_grid(const StringArray &, Grid &);


////////////////////////////////////////////////////////////////////////


bool find_grid_by_name(const char * name, Grid & g)

{

GridInfo i;
bool status = false;

status = find_grid_by_name(name, i);

if ( !status || !(i.ok()) )  return ( false );

status = false;

if ( i.lc )  { g.set( *(i.lc) );  status = true; }
if ( i.st )  { g.set( *(i.st) );  status = true; }
if ( i.ll )  { g.set( *(i.ll) );  status = true; }
if ( i.m  )  { g.set( *(i.m)  );  status = true; }
if ( i.g  )  { g.set( *(i.g)  );  status = true; }

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool find_grid_by_name(const char * name, GridInfo & i)

{

int j;

i.clear();

   //
   //  try dtc lambert grids
   //

for (j=0; j<n_dtc_lambert_grids; ++j)  {

   if ( strcmp(name, dtc_lambert_grids[j].name) == 0 )  {

      i.set( dtc_lambert_grids[j] );

      return ( true );

   }

}

   //
   //  try ncep latlon grids
   //

for (j=0; j<n_ncep_latlon_grids; ++j)  {

   if ( strcmp(name, ncep_latlon_grids[j].name) == 0 )  {

      i.set( ncep_latlon_grids[j] );

      return ( true );

   }

}

   //
   //  try ncep stereographic grids
   //

for (j=0; j<n_ncep_st_grids; ++j)  {

   if ( strcmp(name, ncep_st_grids[j].name) == 0 )  {

      i.set( ncep_st_grids[j] );

      return ( true );

   }

}

   //
   //  try misc stereographic grids
   //

for (j=0; j<n_misc_st_grids; ++j)  {

   if ( strcmp(name, misc_st_grids[j].name) == 0 )  {

      i.set( misc_st_grids[j] );

      return ( true );

   }

}

   //
   //  try ncep lambert grids
   //

for (j=0; j<n_ncep_lambert_grids; ++j)  {

   if ( strcmp(name, ncep_lambert_grids[j].name) == 0 )  {

      i.set( ncep_lambert_grids[j] );

      return ( true );

   }

}

   //
   //  try ncep mercator grids
   //

for (j=0; j<n_ncep_mercator_grids; ++j)  {

   if ( strcmp(name, ncep_mercator_grids[j].name) == 0 )  {

      i.set( ncep_mercator_grids[j] );

      return ( true );

   }

}

   //
   //  try ncep gaussian grids
   //

for (j=0; j<n_ncep_gaussian_grids; ++j)  {

   if ( strcmp(name, ncep_gaussian_grids[j].name) == 0 )  {

      i.set( ncep_gaussian_grids[j] );

      return ( true );

   }

}

   //
   //  nope
   //

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool parse_grid_def(const StringArray &grid_strings, Grid & g)

{

bool status = false;

   //
   //  parse supported projection types
   //

     if ( strcasecmp(grid_strings[0].c_str(), "lambert")   == 0 )  status = parse_lambert_grid(grid_strings, g);
else if ( strcasecmp(grid_strings[0].c_str(), "latlon")    == 0 )  status = parse_latlon_grid(grid_strings, g);
else if ( strcasecmp(grid_strings[0].c_str(), "rotlatlon") == 0 )  status = parse_rotlatlon_grid(grid_strings, g);
else if ( strcasecmp(grid_strings[0].c_str(), "stereo")    == 0 )  status = parse_stereographic_grid(grid_strings, g);
else if ( strcasecmp(grid_strings[0].c_str(), "mercator")  == 0 )  status = parse_mercator_grid(grid_strings, g);
else if ( strcasecmp(grid_strings[0].c_str(), "gaussian")  == 0 )  status = parse_gaussian_grid(grid_strings, g);
else                                                               status = false;

   //
   //  done
   //

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool parse_lambert_grid(const StringArray &grid_strings, Grid & g)

{

Grid * ToGrid = (Grid *) 0;

LambertData ldata;

const int N = grid_strings.n_elements();

if ( (N < 10) || (N > 11) )  {

   mlog << Error << "\nparse_lambert_grid() -> "
        << "lambert conformal grid spec should have 10 or 11 entries\n\n";

   exit ( 1 );

}

int j;
int Nx, Ny;
double lat_ll, lon_ll, lon_orient, D_km, R_km, phi_1, phi_2;
char H;
const char * c = (const char *) 0;

j = 1;

   //
   //  get info from the strings
   //

Nx         = atoi(grid_strings[j++].c_str());
Ny         = atoi(grid_strings[j++].c_str());

lat_ll     = atof(grid_strings[j++].c_str());
lon_ll     = atof(grid_strings[j++].c_str());

lon_orient = atof(grid_strings[j++].c_str());

D_km       = atof(grid_strings[j++].c_str());
R_km       = atof(grid_strings[j++].c_str());

phi_1      = atof(grid_strings[j++].c_str());

if ( N == 11 )  phi_2 = atof(grid_strings[j++].c_str());
else            phi_2 = phi_1;

c          = grid_strings[j++].c_str();

if ( m_strlen(c) != 1 )  {

   mlog << Error << "\nparse_lambert_grid() -> "
        << "bad hemisphere in grid spec\n\n";

   exit ( 1 );

}

H = *c;

if ( (H != 'N') && (H != 'S') )  {

   mlog << Error << "\nparse_lambert_grid() -> "
        << "bad hemisphere in grid spec\n\n";

   exit ( 1 );

}

   //
   //  load up the struct
   //

ldata.name = "To (lambert)";

ldata.nx = Nx;
ldata.ny = Ny;

ldata.hemisphere = H;

ldata.scale_lat_1 = phi_1;
ldata.scale_lat_2 = phi_2;

ldata.lat_pin = lat_ll;
ldata.lon_pin = lon_ll;

ldata.x_pin = 0.0;
ldata.y_pin = 0.0;

ldata.lon_orient = lon_orient;

ldata.d_km = D_km;
ldata.r_km = R_km;

ldata.so2_angle = 0.0;

if ( !west_longitude_positive )  {

   ldata.lon_pin    *= -1.0;
   ldata.lon_orient *= -1.0;

}

ToGrid = new Grid ( ldata );

g = *ToGrid;

if ( ToGrid )  { delete ToGrid; ToGrid = (Grid *) 0; }

   //
   //  done
   //


return ( true );

}


////////////////////////////////////////////////////////////////////////


bool parse_stereographic_grid(const StringArray &grid_strings, Grid & g)

{

Grid * ToGrid = (Grid *) 0;

StereographicData sdata;

const int N = grid_strings.n_elements();

if ( N != 10 )  {

   mlog << Warning << "\nparse_stereographic_grid() -> "
        << "polar stereographic grid spec should have 10 entries\n\n";

   return ( false );

}

int j;
int Nx, Ny;
double lat_ll, lon_ll, lon_orient, D_km, R_km, lat_scale;
char H;
const char * c = (const char *) 0;


j = 1;


   //
   //  get info from the strings
   //

Nx         = atoi(grid_strings[j++].c_str());
Ny         = atoi(grid_strings[j++].c_str());

lat_ll     = atof(grid_strings[j++].c_str());
lon_ll     = atof(grid_strings[j++].c_str());

lon_orient = atof(grid_strings[j++].c_str());

D_km       = atof(grid_strings[j++].c_str());
R_km       = atof(grid_strings[j++].c_str());

lat_scale  = atof(grid_strings[j++].c_str());

c          = grid_strings[j++].c_str();

if ( m_strlen(c) != 1 )  {

   mlog << Error << "\nparse_stereographic_grid() -> "
        << "bad hemisphere in grid spec\n\n";

   exit ( 1 );

}

H = *c;

if ( (H != 'N') && (H != 'S') )  {

   mlog << Error << "\nparse_stereographic_grid() -> "
        << "bad hemisphere in grid spec\n\n";

   exit ( 1 );

}

   //
   //  load up the struct
   //

sdata.name = "To (stereographic)";

sdata.hemisphere = H;

sdata.scale_lat = lat_scale;

sdata.lat_pin = lat_ll;
sdata.lon_pin = lon_ll;

sdata.x_pin = 0.0;
sdata.y_pin = 0.0;

sdata.lon_orient = lon_orient;

sdata.d_km = D_km;
sdata.r_km = R_km;

sdata.nx = Nx;
sdata.ny = Ny;

if ( !west_longitude_positive )  {

   sdata.lon_pin    *= -1.0;
   sdata.lon_orient *= -1.0;

}

sdata.eccentricity = 0.;
sdata.false_east = 0.;
sdata.false_north = 0.;
sdata.scale_factor = 1.0;
sdata.dy_km = sdata.d_km;

ToGrid = new Grid ( sdata );

g = *ToGrid;

if ( ToGrid )  { delete ToGrid; ToGrid = (Grid *) 0; }

   //
   //  done
   //


return ( true );

}


////////////////////////////////////////////////////////////////////////


bool parse_latlon_grid(const StringArray &grid_strings, Grid & g)

{

Grid * ToGrid = (Grid *) 0;

LatLonData ldata;

const int N = grid_strings.n_elements();

if ( N != 7 )  {

   mlog << Error << "\nparse_latlon_grid() -> "
        << "latlon grid spec should have 7 entries\n\n";

   exit ( 1 );

}

int j;
int Nx, Ny;
double lat_ll, lon_ll, delta_lat, delta_lon;


j = 1;

   //
   //  get info from the strings
   //

Nx        = atoi(grid_strings[j++].c_str());
Ny        = atoi(grid_strings[j++].c_str());

lat_ll    = atof(grid_strings[j++].c_str());
lon_ll    = atof(grid_strings[j++].c_str());

delta_lat = atof(grid_strings[j++].c_str());
delta_lon = atof(grid_strings[j++].c_str());

   //
   //  load up the struct
   //

ldata.name = "To (latlon)";

ldata.lat_ll = lat_ll;
ldata.lon_ll = lon_ll;

ldata.delta_lat = delta_lat;
ldata.delta_lon = delta_lon;

ldata.Nlat = Ny;
ldata.Nlon = Nx;

if ( !west_longitude_positive )  {

   ldata.lon_ll *= -1.0;

}

ToGrid = new Grid ( ldata );

g = *ToGrid;

if ( ToGrid )  { delete ToGrid; ToGrid = (Grid *) 0; }


   //
   //  done
   //


return ( true );

}


////////////////////////////////////////////////////////////////////////


bool parse_rotlatlon_grid(const StringArray &grid_strings, Grid & g)

{

Grid * ToGrid = (Grid *) 0;

RotatedLatLonData rdata;

const int N = grid_strings.n_elements();

if ( N != 10 )  {

   mlog << Error << "\nparse_rotlatlon_grid() -> "
        << "rotatedlatlon grid spec should have 10 entries\n\n";

   exit ( 1 );

}

int j;
int Nx, Ny;
double lat_ll, lon_ll, delta_lat, delta_lon;
double true_lat_sp, true_lon_sp, aux_rot;


j = 1;

   //
   //  get info from the strings
   //

Nx          = atoi(grid_strings[j++].c_str());
Ny          = atoi(grid_strings[j++].c_str());

lat_ll      = atof(grid_strings[j++].c_str());
lon_ll      = atof(grid_strings[j++].c_str());

delta_lat   = atof(grid_strings[j++].c_str());
delta_lon   = atof(grid_strings[j++].c_str());

true_lat_sp = atof(grid_strings[j++].c_str());
true_lon_sp = atof(grid_strings[j++].c_str());
aux_rot     = atof(grid_strings[j++].c_str());

   //
   //  load up the struct
   //

rdata.name = "To (rotlatlon)";

rdata.rot_lat_ll = lat_ll;
rdata.rot_lon_ll = lon_ll;

rdata.delta_rot_lat = delta_lat;
rdata.delta_rot_lon = delta_lon;

rdata.Nlat = Ny;
rdata.Nlon = Nx;

rdata.true_lat_south_pole = true_lat_sp;
rdata.true_lon_south_pole = true_lon_sp;

rdata.aux_rotation = aux_rot;

if ( !west_longitude_positive )  {

   rdata.rot_lon_ll *= -1.0;
   rdata.true_lon_south_pole *= -1.0;

}

ToGrid = new Grid ( rdata );

g = *ToGrid;

if ( ToGrid )  { delete ToGrid; ToGrid = (Grid *) 0; }


   //
   //  done
   //


return ( true );

}


////////////////////////////////////////////////////////////////////////


bool parse_mercator_grid(const StringArray &grid_strings, Grid & g)

{

Grid * ToGrid = (Grid *) 0;

MercatorData mdata;

const int N = grid_strings.n_elements();

if ( N != 7 )  {

   mlog << Error << "\nparse_mercator_grid() -> "
        << "mercator grid spec should have 7 entries\n\n";

   exit ( 1 );

}

int j;
int Nx, Ny;
double lat_ll, lon_ll, lat_ur, lon_ur;


j = 1;

   //
   //  get info from the strings
   //

Nx     = atoi(grid_strings[j++].c_str());
Ny     = atoi(grid_strings[j++].c_str());

lat_ll = atof(grid_strings[j++].c_str());
lon_ll = atof(grid_strings[j++].c_str());

lat_ur = atof(grid_strings[j++].c_str());
lon_ur = atof(grid_strings[j++].c_str());


   //
   //  load up the struct
   //

mdata.name = "To (mercator)";

mdata.lat_ll = lat_ll;
mdata.lon_ll = lon_ll;

mdata.lat_ur = lat_ur;
mdata.lon_ur = lon_ur;

mdata.nx = Nx;
mdata.ny = Ny;

if ( !west_longitude_positive )  {

   mdata.lon_ll *= -1.0;
   mdata.lon_ur *= -1.0;

}

ToGrid = new Grid ( mdata );

g = *ToGrid;

if ( ToGrid )  { delete ToGrid; ToGrid = (Grid *) 0; }

   //
   //  done
   //


return ( true );

}


////////////////////////////////////////////////////////////////////////


bool parse_gaussian_grid(const StringArray &grid_strings, Grid & g)

{

Grid * ToGrid = (Grid *) 0;

GaussianData gdata;

const int N = grid_strings.n_elements();

if ( N != 4 )  {

   mlog << Error << "\nparse_gaussian_grid() -> "
        << "gaussian grid spec should have 4 entries\n\n";

   exit ( 1 );

}

int j;
int Nx, Ny;
double lon_zero;


j = 1;

   //
   //  get info from the strings
   //

lon_zero = atof(grid_strings[j++].c_str());
Nx       = atoi(grid_strings[j++].c_str());
Ny       = atoi(grid_strings[j++].c_str());

   //
   //  load up the struct
   //

gdata.name = "To (gaussian)";

gdata.lon_zero = lon_zero;
gdata.nx       = Nx;
gdata.ny       = Ny;

if ( !west_longitude_positive )  {

   gdata.lon_zero *= -1.0;

}

ToGrid = new Grid ( gdata );

g = *ToGrid;

if ( ToGrid )  { delete ToGrid; ToGrid = (Grid *) 0; }

   //
   //  done
   //


return ( true );

}


////////////////////////////////////////////////////////////////////////
