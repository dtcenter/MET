

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "wwmca_ref.h"
#include "interp_base.h"
#include "ave_interp.h"
#include "max_interp.h"
#include "min_interp.h"
#include "nearest_interp.h"
#include "gridhemisphere_to_string.h"
#include "wwmca_grids.h"

#include "vx_met_util/grid_output.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class WwmcaRegridder
   //


////////////////////////////////////////////////////////////////////////


WwmcaRegridder::WwmcaRegridder()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


WwmcaRegridder::~WwmcaRegridder()

{

clear();

if ( NHgrid )  { delete NHgrid;  NHgrid = (const Grid *) 0; }
if ( SHgrid )  { delete SHgrid;  SHgrid = (const Grid *) 0; }

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::init_from_scratch()

{

NHgrid = new Grid (wwmca_north_data);
SHgrid = new Grid (wwmca_south_data);

nh = (const AfwaCloudPctFile *) 0;
sh = (const AfwaCloudPctFile *) 0;

ToGrid = (const Grid *) 0;

interp = (Interpolator *) 0;

Config = (WwmcaConfig *) 0;



clear();

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::clear()

{

// if ( NHgrid )  { delete NHgrid;  NHgrid = (const Grid *) 0; }
// if ( SHgrid )  { delete SHgrid;  SHgrid = (const Grid *) 0; }

if ( nh )  { delete nh;  nh = (const AfwaCloudPctFile *) 0; }
if ( sh )  { delete sh;  sh = (const AfwaCloudPctFile *) 0; }

if ( ToGrid )  { delete ToGrid;  ToGrid = (const Grid *) 0; }

if ( interp )  { delete interp;  interp = (Interpolator *) 0; }

Hemi = no_hemisphere;

grid_strings.clear();

Config = (WwmcaConfig *) 0;

ConfigFilename.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::dump(ostream & out, int depth) const

{

Indent prefix(depth);
char junk[256];

out << prefix << "grid_strings ...\n";

grid_strings.dump(out, depth + 1);

out << prefix << '\n';

gridhemisphere_to_string(Hemi, junk);

out << prefix << "Hemi = " << junk << '\n';

if ( NHgrid )  {

   out << prefix << "NHgrid ...\n";

   NHgrid->dump(out, depth + 1);

} else out << prefix << "NHgrid = (nul)\n";

if ( SHgrid )  {

   out << prefix << "SHgrid ...\n";

   SHgrid->dump(out, depth + 1);

} else out << prefix << "SHgrid = (nul)\n";

if ( nh )  out << prefix << "nh set\n";
else       out << prefix << "nh not set\n";

if ( sh )  out << prefix << "sh set\n";
else       out << prefix << "sh not set\n";

if ( ToGrid ) {

   out << prefix << "ToGrid ...\n";

   ToGrid->dump(out, depth + 1);

} else out << prefix << "ToGrid = (nul)\n";



if ( interp ) {

   out << prefix << "interp ...\n";

   interp->dump(out, depth + 1);

} else out << prefix << "interp = (nul)\n";

out << prefix << "ConfigFilename = ";

if ( ConfigFilename.length() == 0 )  out << "(nul)\n";
else                                 out << '\"' << ConfigFilename << "\"\n\n";

   //
   // done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::set_nh_file(const char * filename)

{

AfwaCloudPctFile * f = new AfwaCloudPctFile;

if ( !(f->read(filename)) )  {

   cerr << "\n\n  WwmcaRegridder::set_nh_file(const char *) -> unable to open afwa cloud pct file \"" << filename << "\"\n\n";

   exit ( 1 );

}

nh = (const AfwaCloudPctFile *) f;  f = (AfwaCloudPctFile *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::set_sh_file(const char * filename)

{

AfwaCloudPctFile * f = new AfwaCloudPctFile;

if ( !(f->read(filename)) )  {

   cerr << "\n\n  WwmcaRegridder::set_sh_file(const char *) -> unable to open afwa cloud pct file \"" << filename << "\"\n\n";

   exit ( 1 );

}

sh = (const AfwaCloudPctFile *) f;  f = (AfwaCloudPctFile *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::set_config(WwmcaConfig & wc, const char * config_filename)

{

Config = &wc;

ConfigFilename = config_filename;

   //
   //  set interpolator
   //

get_interpolator();

if ( !interp )  {

   cerr << "\n\n  WwmcaRegridder::set_config(WwmcaConfig &) -> bad interpolator specification in config file\"" << ConfigFilename << "\"\n\n";

   exit ( 1 );

}

   //
   //  set "to" grid
   //

get_grid();

if ( !ToGrid )  {

   cerr << "\n\n  WwmcaRegridder::set_config(WwmcaConfig &) -> bad \"to\" grid specification in config file\"" << ConfigFilename << "\"\n\n";

   exit ( 1 );

}


   //
   //  find grid hemisphere
   //

find_grid_hemisphere();


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


InterpolationValue WwmcaRegridder::operator()(int x, int y) const

{

InterpolationValue value;

value = get_interpolated_value(x, y);

return ( value );

}


////////////////////////////////////////////////////////////////////////


InterpolationValue WwmcaRegridder::get_interpolated_value(int to_x, int to_y) const

{

char junk[256];
InterpolationValue value;


switch ( Hemi )  {

   case north_hemisphere:
      value = do_single_hemi(to_x, to_y, NHgrid, *nh);
      break;

   case south_hemisphere:
      value = do_single_hemi(to_x, to_y, SHgrid, *sh);
      break;

   case both_hemispheres:
      value = do_both_hemi(to_x, to_y);
      break;

   default:
      gridhemisphere_to_string(Hemi, junk);
      cerr << "\n\n  WwmcaRegridder::get_interpolated_value(int x, int y) const -> bad hemisphere ... " << junk << "\n\n";
      exit ( 1 );
      break;

}   //  switch


return ( value );

}


////////////////////////////////////////////////////////////////////////


InterpolationValue WwmcaRegridder::do_single_hemi(int to_x0, int to_y0, const Grid * From, const AfwaCloudPctFile & cloud) const

{

Interpolator & I = *interp;
InterpolationValue iv;
int from_x0, from_y0, from_x, from_y;
int sub_x, sub_y;
int wm1o2;
int xx, yy;
double lat, lon, dx, dy, t;



   //
   //  load interpolation subgrid
   //

ToGrid->xy_to_latlon((double) to_x0, (double) to_y0, lat, lon);

From->latlon_to_xy(lat, lon, dx, dy);

from_x0 = nint(dx);
from_y0 = nint(dy);

   //
   //  Width = 1?  then do nearest neighbor interpolation
   //

if ( I.width() == 1 )  {

   if ( cloud.xy_is_ok(from_x0, from_y0) )  {

      iv.ok = true;

      iv.value = (double) (cloud(from_x0, from_y0));

   } else {

      iv.ok = false;

      iv.value = 0.0;

   }

   return ( iv );

}

   //
   //  Width > 1
   //

wm1o2 = I.wm1o2();

for (xx=-wm1o2; xx<=wm1o2; ++xx)  {

   from_x = xx + from_x0;

   sub_x  = xx + wm1o2;

   for (yy=-wm1o2; yy<=wm1o2; ++yy)  {

      from_y = yy + from_y0;

      sub_y  = yy + wm1o2;

      if ( !(cloud.xy_is_ok(from_x, from_y)) )  {

         I.put_bad(sub_x, sub_y);

      } else {

         t = (double) (cloud(from_x, from_y));

         I.put_good(sub_x, sub_y, t);

      }
      
   }   //  for yy

}   //  for xx

   //
   //  get interpolated value
   //

iv = I(dx - from_x0, dy - from_y0);

   //
   //  done
   //

return ( iv );

}


////////////////////////////////////////////////////////////////////////


InterpolationValue WwmcaRegridder::do_both_hemi(int to_x0, int to_y0) const

{

Interpolator & I = *interp;
double lat0, lon0, lat, lon;
double dx, dy, dx0, dy0;
double t;
int from_x0, from_y0;
int xx, yy, sub_x, sub_y, from_x, from_y, wm1o2;
InterpolationValue iv;
const AfwaCloudPctFile  * cloud_this  = (const AfwaCloudPctFile *) 0;
const AfwaCloudPctFile  * cloud_other = (const AfwaCloudPctFile *) 0;
const Grid * From_this  = (const Grid *) 0;
const Grid * From_other = (const Grid *) 0;



   //
   //  load interpolation subgrid
   //

ToGrid->xy_to_latlon((double) to_x0, (double) to_y0, lat0, lon0);

if ( lat0 >= 0.0 )  {

   cloud_this  = nh;
   cloud_other = sh;

   From_this  = NHgrid;
   From_other = SHgrid;

} else {

   cloud_this  = sh;
   cloud_other = nh;

   From_this  = SHgrid;
   From_other = NHgrid;

}

From_this->latlon_to_xy(lat0, lon0, dx0, dy0);

from_x0 = nint(dx0);
from_y0 = nint(dy0);


   //
   //  Width = 1?  then do nearest neighbor interpolation
   //

if ( I.width() == 1 )  {

   if ( cloud_this->xy_is_ok(from_x0, from_y0) )  {

      iv.ok = true;

      iv.value = (double) (cloud_this->get_value(from_x0, from_y0));

   } else {

      iv.ok = false;

      iv.value = 0.0;

   }

   return ( iv );

}   //  if Width == 1

   //
   //  Width > 1
   //

wm1o2 = I.wm1o2();

for (xx=-wm1o2; xx<=wm1o2; ++xx)  {

   from_x = xx + from_x0;

   sub_x  = xx + wm1o2;

   for (yy=-wm1o2; yy<=wm1o2; ++yy)  {

      from_y = yy + from_y0;

      sub_y  = yy + wm1o2;

      From_this->xy_to_latlon((double) from_x, (double) from_y, lat, lon);

      if ( lat*lat0 >= 0 )  {   //  same hemisphere

         t = cloud_this->get_value(from_x, from_y);

      } else {

         From_other->latlon_to_xy(lat, lon, dx, dy);

         t = cloud_other->get_value(nint(dx), nint(dy));

      }

      I.put_good(sub_x, sub_y, t);

   }   //  for yy

}   //  for xx


   //
   //  get interpolated value
   //

iv = I(dx0 - from_x0, dy0 - from_y0);

   //
   //  done
   //

return ( iv );

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::find_grid_hemisphere()

{

int j, ix, iy;
double x, y, lat, lon;
bool nh_used    = false;
bool sh_used    = false;
const int Nx    = ToGrid->nx();
const int Ny    = ToGrid->ny();
const int Width = Config->interp_width();


Hemi = no_hemisphere;

   //
   //  bottom, top
   //

for (j=0; j<Nx; ++j)  {

   x = (double) j;

   ToGrid->xy_to_latlon(x, 0.0, lat, lon);

   if ( lat > 0.0 )  nh_used = true;
   if ( lat < 0.0 )  sh_used = true;

   ToGrid->xy_to_latlon(x, Ny - 1.0, lat, lon);

   if ( lat > 0.0 )  nh_used = true;
   if ( lat < 0.0 )  sh_used = true;

}

if ( nh && sh )  { Hemi = both_hemispheres;  return; }

   //
   //  left, right
   //

for (j=0; j<Ny; ++j)  {

   y = (double) j;

   ToGrid->xy_to_latlon(0.0, y, lat, lon);

   if ( lat > 0.0 )  nh_used = true;
   if ( lat < 0.0 )  sh_used = true;

   ToGrid->xy_to_latlon(Nx - 1.0, y, lat, lon);

   if ( lat > 0.0 )  nh_used = true;
   if ( lat < 0.0 )  sh_used = true;

}

if ( nh && sh )  { Hemi = both_hemispheres;  return; }

if ( Width == 1 )  {

   if ( nh_used )  Hemi = north_hemisphere;
   if ( sh_used )  Hemi = south_hemisphere;

   return;

}

   //
   //  around the equator
   //

const int wm102 = (Width - 1)/2;
const int xmin  = -wm102;
const int xmax  = Nx - 1 + wm102;
const int ymin  = -wm102;
const int ymax  = Ny - 1 + wm102;

for (j=0; j<720; ++j)  {

   lat = 0.0;

   lon = 0.5*j;   //  check every half degree

   ToGrid->latlon_to_xy(lat, lon, x, y);

   ix = nint(x);
   iy = nint(y);

   if ( (ix >= xmin) && (ix <= xmax) && (iy >= ymin) && (iy <= ymax) )  {

      Hemi = both_hemispheres;

      return;

   }

}   //  for j


   //
   //  done
   //

if ( nh_used )  Hemi = north_hemisphere;
if ( sh_used )  Hemi = south_hemisphere;

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::get_interpolator()

{

int width, good_percent;
int n_good_needed;
ConcatString method;
Ave_Interp ave;
Nearest_Interp nearest;
Min_Interp mini;
Max_Interp maxi;
Result r;


width = Config->interp_width();

good_percent = Config->good_percent();

n_good_needed = nint(ceil(0.01*good_percent*width));

r = Config->interp_method();

method = r.sval();


   //
   //  get to work
   //

if ( method == "average" )  {

   ave.set_size(width);

   ave.set_ngood_needed(n_good_needed);

   interp = ave.copy();

   return;

}


if ( method == "nearest" )  {

   nearest.set_size(width);

   nearest.set_ngood_needed(n_good_needed);

   interp = nearest.copy();

   return;

}

if ( method == "min" )  {

   mini.set_size(width);

   mini.set_ngood_needed(n_good_needed);

   interp = mini.copy();

   return;

}


if ( method == "max" )  {

   maxi.set_size(width);

   maxi.set_ngood_needed(n_good_needed);

   interp = maxi.copy();

   return;

}



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::get_grid()

{

int n;
char * line = (char *) 0;
char * c = (char *) 0;
char * p = (char *) 0;
const char delim [] = " ";
ConcatString s;
ConcatString gridinfo_string;
Result r;
bool status = false;
Grid * G = (Grid *) 0;




r = Config->To_Grid();

gridinfo_string = r.sval();

grid_strings.clear();

n = 1 + gridinfo_string.length();

line = new char [n];

memset(line, 0, n);

strcpy(line, (const char *) gridinfo_string);

p = line;

while ( (c = strtok(p, delim)) != 0 )  {

   grid_strings.add(c);

   p = (char *) 0;

}   //  while

s = grid_strings[0];

   //
   //  if only one string in array, lookup by name
   //

if ( grid_strings.n_elements() == 1 )  {

   status = find_grid_by_name(grid_strings[0], ginfo);

   if ( !status || !(ginfo.ok()) )  {

      cerr << "\n\n  WwmcaRegridder::get_grid() -> can't find any grid with name \"" << grid_strings[0] << "\"\n\n";

      exit ( 1 );

   }

   G = new Grid;

   if ( ginfo.lc )  {  G->set( *(ginfo.lc) );  }
   if ( ginfo.st )  {  G->set( *(ginfo.st) );  }
   if ( ginfo.ll )  {  G->set( *(ginfo.ll) );  }
   if ( ginfo.m  )  {  G->set( *(ginfo.m)  );  }

   ToGrid = (const Grid *) G;

   return;

}

   //
   //  lambert?
   //

     if ( s == "lambert"  )  parse_lambert_grid();
else if ( s == "latlon"   )  parse_latlon_grid();
else if ( s == "stereo"   )  parse_stereographic_grid();
else if ( s == "mercator" )  parse_mercator_grid();
else {

cerr << "\n\n  WwmcaRegridder::get_grid() -> can't create grid from config file string \""
     << gridinfo_string << "\"\n\n";

exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::parse_lambert_grid()

{

if ( ToGrid )  { delete ToGrid;  ToGrid = (const Grid *) 0; }

LambertData * ldata = new LambertData;

const int N = grid_strings.n_elements();

if ( (N < 9) || (N > 10) )  {

   cerr << "\n\n  WwmcaRegridder::parse_lambert_grid() -> bad grid spec in config file \"" << ConfigFilename << "\"\n\n";

   exit ( 1 );

}

int j;
int Nx, Ny;
double lat_ll, lon_ll, lon_orient, D_km, R_km, phi_1, phi_2;

j = 1;

   //
   //  get info from the strings
   //

Nx         = atoi(grid_strings[j++]);
Ny         = atoi(grid_strings[j++]);

lat_ll     = atof(grid_strings[j++]);
lon_ll     = atof(grid_strings[j++]);

lon_orient = atof(grid_strings[j++]);

D_km       = atof(grid_strings[j++]);
R_km       = atof(grid_strings[j++]);

phi_1      = atof(grid_strings[j++]);

if ( N == 10 )  phi_2 = atof(grid_strings[j++]);
else            phi_2 = phi_1;

   //
   //  load up the struct
   //

ldata->name = "To (lambert)";

ldata->nx = Nx;
ldata->ny = Ny;

ldata->scale_lat_1 = phi_1;
ldata->scale_lat_2 = phi_2;

ldata->lat_pin = lat_ll;
ldata->lon_pin = lon_ll;

ldata->x_pin = 0.0;
ldata->y_pin = 0.0;

ldata->lon_orient = lon_orient;

ldata->d_km = D_km;
ldata->r_km = R_km;


if ( !west_longitude_positive )  {

   ldata->lon_pin    *= -1.0;
   ldata->lon_orient *= -1.0;

}

ToGrid = new Grid ( *(ldata) );

   //
   //  done
   //

ginfo.clear();

ginfo.set(*ldata);

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::parse_stereographic_grid()

{

if ( ToGrid )  { delete ToGrid;  ToGrid = (const Grid *) 0; }

StereographicData * sdata = new StereographicData;

const int N = grid_strings.n_elements();

if ( N != 10 )  {

   cerr << "\n\n  WwmcaRegridder::parse_stereographic_grid() -> bad grid spec in config file \"" << ConfigFilename << "\"\n\n";

   exit ( 1 );

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

Nx         = atoi(grid_strings[j++]);
Ny         = atoi(grid_strings[j++]);

lat_ll     = atof(grid_strings[j++]);
lon_ll     = atof(grid_strings[j++]);

lon_orient = atof(grid_strings[j++]);

D_km       = atof(grid_strings[j++]);
R_km       = atof(grid_strings[j++]);

lat_scale  = atof(grid_strings[j++]);

c          = grid_strings[j++];

if ( strlen(c) != 1 )  {

   cerr << "\n\n  WwmcaRegridder::parse_stereographic_grid() -> bad hemisphere in grid spec in config file \"" << ConfigFilename << "\"\n\n";

   exit ( 1 );

}

H = *c;

if ( (H != 'N') && (H != 'S') )  {

   cerr << "\n\n  WwmcaRegridder::parse_stereographic_grid() -> bad hemisphere in grid spec in config file \"" << ConfigFilename << "\"\n\n";

   exit ( 1 );

}

if ( H == 'S' )  {

   cerr << "\n\n  WwmcaRegridder::parse_stereographic_grid() -> South hemisphere grids not yet supported\n\n";

   exit ( 1 );

}


   //
   //  load up the struct
   //

sdata->name = "To (stereographic)";

sdata->scale_lat = lat_scale;

sdata->lat_pin = lat_ll;
sdata->lon_pin = lon_ll;

sdata->x_pin = 0.0;
sdata->y_pin = 0.0;

sdata->lon_orient = lon_orient;

sdata->d_km = D_km;
sdata->r_km = R_km;

sdata->nx = Nx;
sdata->ny = Ny;

if ( !west_longitude_positive )  {

   sdata->lon_pin    *= -1.0;
   sdata->lon_orient *= -1.0;

}

ToGrid = new Grid ( *(sdata) );

   //
   //  done
   //

ginfo.clear();

ginfo.set(*sdata);

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::parse_latlon_grid()

{

if ( ToGrid )  { delete ToGrid;  ToGrid = (const Grid *) 0; }

LatLonData * ldata = new LatLonData;

const int N = grid_strings.n_elements();

if ( N != 7 )  {

   cerr << "\n\n  WwmcaRegridder::parse_latlon_grid() -> bad grid spec in config file \"" << ConfigFilename << "\"\n\n";

   exit ( 1 );

}

int j;
int Nx, Ny;
double lat_ll, lon_ll, delta_lat, delta_lon;


j = 1;

   //
   //  get info from the strings
   //

Nx        = atoi(grid_strings[j++]);
Ny        = atoi(grid_strings[j++]);

lat_ll    = atof(grid_strings[j++]);
lon_ll    = atof(grid_strings[j++]);

delta_lat = atof(grid_strings[j++]);
delta_lon = atof(grid_strings[j++]);

   //
   //  load up the struct
   //

ldata->name = "To (latlon)";

ldata->lat_ll = lat_ll;
ldata->lon_ll = lon_ll;

ldata->delta_lat = delta_lat;
ldata->delta_lon = delta_lon;

ldata->Nlat = Ny;
ldata->Nlon = Nx;

if ( !west_longitude_positive )  {

   ldata->lon_ll    *= -1.0;
   ldata->delta_lon *= -1.0;

}

ToGrid = new Grid ( *(ldata) );

   //
   //  done
   //

ginfo.clear();

ginfo.set(*ldata);

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::parse_mercator_grid()

{

if ( ToGrid )  { delete ToGrid;  ToGrid = (const Grid *) 0; }

MercatorData * mdata = new MercatorData;

const int N = grid_strings.n_elements();

if ( N != 7 )  {

   cerr << "\n\n  WwmcaRegridder::parse_mercator_grid() -> bad grid spec in config file \"" << ConfigFilename << "\"\n\n";

   exit ( 1 );

}

int j;
int Nx, Ny;
double lat_ll, lon_ll, lat_ur, lon_ur;


j = 1;

   //
   //  get info from the strings
   //

Nx     = atoi(grid_strings[j++]);
Ny     = atoi(grid_strings[j++]);

lat_ll = atof(grid_strings[j++]);
lon_ll = atof(grid_strings[j++]);

lat_ur = atof(grid_strings[j++]);
lon_ur = atof(grid_strings[j++]);


   //
   //  load up the struct
   //

mdata->name = "To (mercator)";

mdata->lat_ll = lat_ll;
mdata->lon_ll = lon_ll;

mdata->lat_ur = lat_ur;
mdata->lon_ur = lon_ur;


if ( !west_longitude_positive )  {

   mdata->lon_ll *= -1.0;
   mdata->lon_ur *= -1.0;

}


ToGrid = new Grid ( *(mdata) );

   //
   //  done
   //

ginfo.clear();

ginfo.set(*mdata);

return;

}


////////////////////////////////////////////////////////////////////////


