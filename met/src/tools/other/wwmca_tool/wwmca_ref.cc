// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "vx_log.h"
#include "vx_grid.h"

#include "wwmca_ref.h"
#include "interp_base.h"
#include "ave_interp.h"
#include "max_interp.h"
#include "min_interp.h"
#include "nearest_interp.h"
#include "gridhemisphere_to_string.h"

#include "grid_output.h"
#include "apply_mask.h"


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

cp_nh = (const AFCloudPctFile *) 0;
cp_sh = (const AFCloudPctFile *) 0;

pt_nh = (const AFPixelTimeFile *) 0;
pt_sh = (const AFPixelTimeFile *) 0;

ToGrid = (const Grid *) 0;

Config = (MetConfig *) 0;

interp_func = 0;



clear();

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::clear()

{

if ( cp_nh )  { delete cp_nh;  cp_nh = (const AFCloudPctFile *) 0; }
if ( cp_sh )  { delete cp_sh;  cp_sh = (const AFCloudPctFile *) 0; }

if ( pt_nh )  { delete pt_nh;  pt_nh = (const AFPixelTimeFile *) 0; }
if ( pt_sh )  { delete pt_sh;  pt_sh = (const AFPixelTimeFile *) 0; }

if ( ToGrid )  { delete ToGrid;  ToGrid = (const Grid *) 0; }

Hemi = no_hemisphere;

grid_strings.clear();

Config = (MetConfig *) 0;

ConfigFilename.clear();

Width = 0;

Method = InterpMthd_None;

interp_func = 0;

Fraction = 0.0;   //  is this a good default value?

WritePixelAge = false;


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

if ( cp_nh )  out << prefix << "nh set\n";
else          out << prefix << "nh not set\n";

if ( cp_sh )  out << prefix << "sh set\n";
else          out << prefix << "sh not set\n";

if ( ToGrid ) {

   out << prefix << "ToGrid ...\n";

   ToGrid->dump(out, depth + 1);

} else out << prefix << "ToGrid = (nul)\n";



out << prefix << "Interpolation Method   = " << interpmthd_to_string(Method) << "\n";
out << prefix << "Interpolation Width    = " << Width    << "\n";
out << prefix << "Interpolation Fraction = " << Fraction << "\n";
out << prefix << "Write Pixel Age        = " << bool_to_string(WritePixelAge) << "\n";


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


void WwmcaRegridder::set_cp_nh_file(const char * filename)

{

AFCloudPctFile * f = new AFCloudPctFile;

if ( !(f->read(filename, 'N')) )  {

   mlog << Error << "\nWwmcaRegridder::set_cp_nh_file(const char *) -> "
        << "unable to open cloud pct file \"" << filename << "\"\n\n";

   exit ( 1 );

}

cp_nh = (const AFCloudPctFile *) f;  f = (AFCloudPctFile *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::set_cp_sh_file(const char * filename)

{

AFCloudPctFile * f = new AFCloudPctFile;

if ( !(f->read(filename, 'S')) )  {

   mlog << Error << "\nWwmcaRegridder::set_cp_sh_file(const char *) -> "
        << "unable to open cloud pct file \"" << filename << "\"\n\n";

   exit ( 1 );

}

cp_sh = (const AFCloudPctFile *) f;  f = (AFCloudPctFile *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::set_pt_nh_file(const char * filename, bool swap)

{

AFPixelTimeFile * f = new AFPixelTimeFile;

if ( !(f->read(filename, 'N')) )  {

   mlog << Error << "\nWwmcaRegridder::set_pt_nh_file(const char *) -> "
        << "unable to open pixel time file \"" << filename << "\"\n\n";

   exit ( 1 );

}

f->set_swap_endian(swap);

pt_nh = (const AFPixelTimeFile *) f;  f = (AFPixelTimeFile *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::set_pt_sh_file(const char * filename, bool swap)

{

AFPixelTimeFile * f = new AFPixelTimeFile;

if ( !(f->read(filename, 'S')) )  {

   mlog << Error << "\nWwmcaRegridder::set_pt_sh_file(const char *) -> "
        << "unable to open pixel time file \"" << filename << "\"\n\n";

   exit ( 1 );

}

f->set_swap_endian(swap);

pt_sh = (const AFPixelTimeFile *) f;  f = (AFPixelTimeFile *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::set_config(MetConfig & wc, const char * config_filename)

{

Config = &wc;

ConfigFilename = config_filename;

   //
   //  set interpolator
   //

const RegridInfo regrid_info = parse_conf_regrid(Config);

Width = regrid_info.width;

Method = InterpMthd_Nearest;

Fraction = regrid_info.vld_thresh;

WritePixelAge = Config->lookup_bool(conf_key_write_pixel_age);

   //
   //  writing pixel age instead of cloud data
   //

if ( WritePixelAge )  {

   mlog << Debug(2)
        << "Writing pixel age times instead of cloud data.\n";

   if( pt_nh == 0 && pt_sh == 0 )  {

      mlog << Error << "\nWwmcaRegridder::set_config() -> "
           << "when the \"" << conf_key_write_pixel_age << "\" configuration option is enabled, "
           << "at least one pixel time file must be specified on the command line.\n\n";

      exit ( 1 );

   }
}

if ( Width > 1 )  {

   Method = regrid_info.method;

   switch ( Method )  {

      case InterpMthd_Min:
         interp_func = &dp_interp_min;
         break;

      case InterpMthd_Max:
         interp_func = &dp_interp_max;
         break;

      case InterpMthd_UW_Mean:
         interp_func = &dp_interp_uw_mean;
         break;

      default:
         mlog << Error
              << "\n\n  WwmcaRegridder::set_config(MetConfig & wc, const char * config_filename) -> "
              << "bad interpolation method ... " << interpmthd_to_string(Method) << "\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  if


   //
   //  set "to" grid
   //

get_grid();

if ( !ToGrid )  {

   mlog << Error << "\nWwmcaRegridder::set_config(wwmca_regrid_Conf &) -> "
        << "bad \"to\" grid specification in config file\""
        << ConfigFilename << "\"\n\n";

   exit ( 1 );

}


   //
   //  find grid hemisphere
   //

find_grid_hemisphere();


   //
   //  check that the hemispheres are defined
   //

if ( ( Hemi == north_hemisphere || Hemi == both_hemispheres ) && !cp_nh ) {

   mlog << Error << "\nWwmcaRegridder::set_config() -> "
        << "missing northern hemisphere data must be specified using the "
        << "\"-nh\" argument\n\n";

   exit ( 1 );

}

if ( ( Hemi == south_hemisphere || Hemi == both_hemispheres ) && !cp_sh ) {

   mlog << Error << "\nWwmcaRegridder::set_config() -> "
        << "missing southern hemisphere data must be specified using the "
        << "\"-sh\" argument\n\n";

   exit ( 1 );

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::get_interpolated_data(DataPlane & dp) const

{

char junk[256];
InterpolationValue value;


switch ( Hemi )  {

   case north_hemisphere:
      do_single_hemi(dp, NHgrid, cp_nh, pt_nh);
      break;

   case south_hemisphere:
      do_single_hemi(dp, SHgrid, cp_sh, pt_sh);
      break;

   case both_hemispheres:
      do_both_hemi(dp);
      break;

   default:
      gridhemisphere_to_string(Hemi, junk);
      mlog << Error << "\nWwmcaRegridder::get_interpolated_data(DataPlane &) const -> "
           << "bad hemisphere ... " << junk << "\n\n";
      exit ( 1 );
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::do_single_hemi(DataPlane & dp, const Grid * From, const AFCloudPctFile * cloud, const AFPixelTimeFile * pixel) const

{

double v;
int from_x, from_y;
int x, y, xx, yy;
double lat, lon, dx, dy;
int pixel_age_minutes;
const double max_minutes = Config->lookup_double(conf_key_max_minutes);


if ( !cloud )  {
   mlog << Error << "\nWwmcaRegridder::do_single_hemi() -> "
        << "null pointers for cloud percent file.\n\n";
   exit ( 1 );
}


dp.set_size(ToGrid->nx(), ToGrid->ny());


   //
   //  Width = 1?  then do nearest neighbor interpolation
   //

if ( Width == 1 )  {

   for (x=0; x<(dp.nx()); ++x)  {

      for (y=0; y<(dp.ny()); ++y)  {

         ToGrid->xy_to_latlon((double) x, (double) y, lat, lon);

         From->latlon_to_xy(lat, lon, dx, dy);

         from_x = nint(dx);
         from_y = nint(dy);

         pixel_age_minutes = 0;

         v = bad_data_double;

         if ( pixel )  pixel_age_minutes = pixel->pixel_age_sec(from_x, from_y) / 60;

         if ( cloud->xy_is_ok(from_x, from_y) && pixel_age_minutes < max_minutes )  {

            if ( WritePixelAge )  v = (double) pixel_age_minutes;
            else                  v = (double) ((*cloud)(from_x, from_y));

         }

         dp.put(v, x, y);

      }   //  for y

   }   //  for x

   return;

}   //  if Width == 1

   //
   //  Width > 1
   //

DataPlane fat;
int x_fat, y_fat;


fat.set_size(Width*(dp.nx()), Width*(dp.ny()));

const int wm1o2 = (Width - 1)/2;

const double t = 1.0/wm1o2;


for (x=0; x<(dp.nx()); ++x)  {

   for (y=0; y<(dp.ny()); ++y)  {

      for (xx=-wm1o2; xx<=wm1o2; ++xx)  {

         x_fat = Width*x + wm1o2 + xx;

         for (yy=-wm1o2; yy<=wm1o2; ++yy)  {

            y_fat = Width*y + wm1o2 + yy;

            ToGrid->xy_to_latlon(x + t*xx, y + t*yy, lat, lon);

            From->latlon_to_xy(lat, lon, dx, dy);

            from_x = nint(dx);
            from_y = nint(dy);

            if ( pixel )  pixel_age_minutes = pixel->pixel_age_sec(from_x, from_y) / 60;
            else          pixel_age_minutes = 0;

            if ( !(cloud->xy_is_ok(from_x, from_y) && pixel_age_minutes < max_minutes) )  {

               v = bad_data_double;

            } else {

              if ( WritePixelAge )  v = pixel_age_minutes;
              else                  v = (double) ((*cloud)(from_x, from_y));

            }

            fat.put(v, x_fat, y_fat);

         }   //  for yy

      }   //  for xx

   }   //  for y

}   //  for x


interp_func(fat, dp, Width, Fraction);












/*
for (xx=-wm1o2; xx<=wm1o2; ++xx)  {

   from_x = xx + from_x0;

   sub_x  = xx + wm1o2;

   for (yy=-wm1o2; yy<=wm1o2; ++yy)  {

      from_y = yy + from_y0;

      sub_y  = yy + wm1o2;

      if ( pixel )  pixel_age_minutes = pixel->pixel_age_sec(from_x0, from_y0) / 60;
      else          pixel_age_minutes = 0;

      if ( !(cloud->xy_is_ok(from_x, from_y) && pixel_age_minutes < max_minutes) )  {

         I.put_bad(sub_x, sub_y);

      } else {

         t = (double) ((*cloud)(from_x, from_y));

         I.put_good(sub_x, sub_y, t);

      }

   }   //  for yy

}   //  for xx

   //
   //  get interpolated value
   //

iv = I(dx - from_x0, dy - from_y0);
*/

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::do_both_hemi(DataPlane & dp) const

{

double lat, lon;
double dx, dy;
double v;
int x, y, xx, yy, from_x, from_y;
const AFCloudPctFile   * cloud = 0;
const AFPixelTimeFile  * pixel = 0;
const Grid             * From  = 0;
int pixel_age_minutes;
const double max_minutes = Config->lookup_double(conf_key_max_minutes);


dp.set_size(ToGrid->nx(), ToGrid->ny());

   //
   //  load interpolation subgrid
   //
/*
ToGrid->xy_to_latlon((double) to_x0, (double) to_y0, lat0, lon0);

if ( lat0 >= 0.0 )  {

   cloud_this  = cp_nh;
   cloud_other = cp_sh;

   pixel_this  = pt_nh;
   pixel_other = pt_sh;

   From_this  = NHgrid;
   From_other = SHgrid;

} else {

   cloud_this  = cp_sh;
   cloud_other = cp_nh;

   pixel_this  = pt_nh;
   pixel_other = pt_sh;

   From_this  = SHgrid;
   From_other = NHgrid;

}

From_this->latlon_to_xy(lat0, lon0, dx0, dy0);

from_x0 = nint(dx0);
from_y0 = nint(dy0);
*/

   //
   //  Width = 1?  then do nearest neighbor interpolation
   //

if ( Width == 1 )  {

   for (x=0; x<(dp.nx()); ++x)  {

      for (y=0; y<(dp.ny()); ++y)  {

         ToGrid->xy_to_latlon((double) x, (double) y, lat, lon);

         if ( lat >= 0.0 )  {

            cloud  = cp_nh;
            pixel  = pt_nh;
            From   = NHgrid;

         } else {

            cloud  = cp_sh;
            pixel  = pt_sh;
            From   = SHgrid;

         }

         From->latlon_to_xy(lat, lon, dx, dy);

         from_x = nint(dx);
         from_y = nint(dy);

         pixel_age_minutes = 0;

         v = bad_data_double;

         if ( pixel )  pixel_age_minutes = pixel->pixel_age_sec(from_x, from_y) / 60;

         if ( cloud->xy_is_ok(from_x, from_y) && pixel_age_minutes < max_minutes )  {

            if ( WritePixelAge )  v = (double) pixel_age_minutes;
            else                  v = (double) cloud->cloud_pct(from_x, from_y);

         }

         dp.put(v, x, y);

      }   //  for y

   }   //  for x

   return;

}   //  if Width == 1

   //
   //  Width > 1
   //


DataPlane fat;
int x_fat, y_fat;


fat.set_size(Width*(dp.nx()), Width*(dp.ny()));

const int wm1o2 = (Width - 1)/2;

const double t = 1.0/wm1o2;


for (x=0; x<(dp.nx()); ++x)  {

   for (y=0; y<(dp.ny()); ++y)  {

      for (xx=-wm1o2; xx<=wm1o2; ++xx)  {

         x_fat = Width*x + wm1o2 + xx;

         for (yy=-wm1o2; yy<=wm1o2; ++yy)  {

            y_fat = Width*y + wm1o2 + yy;

            ToGrid->xy_to_latlon(x + t*xx, y + t*yy, lat, lon);

            if ( lat >= 0.0 )  {

               cloud  = cp_nh;
               pixel  = pt_nh;
               From   = NHgrid;

            } else {

               cloud  = cp_sh;
               pixel  = pt_sh;
               From   = SHgrid;

            }

            From->latlon_to_xy(lat, lon, dx, dy);

            from_x = nint(dx);
            from_y = nint(dy);

            pixel_age_minutes = 0;

            v = bad_data_double;

            if ( pixel )  pixel_age_minutes = pixel->pixel_age_sec(from_x, from_y) / 60;

            if ( cloud->xy_is_ok(from_x, from_y) && pixel_age_minutes < max_minutes )  {

               if ( WritePixelAge )  v = (double) pixel_age_minutes;
               else                  v = (double) cloud->cloud_pct(from_x, from_y);

            }

            fat.put(v, x_fat, y_fat);

         }   //  for yy

      }   //  for xx

   }   //  for y

}   //  for x


interp_func(fat, dp, Width, Fraction);









/*
wm1o2 = I.wm1o2();

for (xx=-wm1o2; xx<=wm1o2; ++xx)  {

   from_x = xx + from_x0;

   sub_x  = xx + wm1o2;

   for (yy=-wm1o2; yy<=wm1o2; ++yy)  {

      from_y = yy + from_y0;

      sub_y  = yy + wm1o2;

      From_this->xy_to_latlon((double) from_x, (double) from_y, lat, lon);

      if ( lat*lat0 >= 0 )  {   //  same hemisphere

         pixel_age_minutes = 0;

         if ( pixel_this )  pixel_age_minutes = pixel_this->pixel_age_sec(from_x, from_y) / 60;

         if (pixel_age_minutes < max_minutes)  {

            t = cloud_this->cloud_pct(from_x, from_y);

            I.put_good(sub_x, sub_y, t);

         } else {

            I.put_bad(sub_x, sub_y);

         }

      } else {

         From_other->latlon_to_xy(lat, lon, dx, dy);

         if ( pixel_other )  pixel_age_minutes = pixel_other->pixel_age_sec(nint(dx), nint(dy)) / 60;
         else                pixel_age_minutes = 0;

         if (pixel_age_minutes < max_minutes)  {

            t = cloud_other->cloud_pct(nint(dx), nint(dy));

            I.put_good(sub_x, sub_y, t);

         } else {

            I.put_bad(sub_x, sub_y);

         }

      }

   }   //  for yy

}   //  for xx


   //
   //  get interpolated value
   //

iv = I(dx0 - from_x0, dy0 - from_y0);
*/

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void WwmcaRegridder::find_grid_hemisphere()

{

int j, ix, iy;
double x, y, lat, lon;
bool nh_used                 = false;
bool sh_used                 = false;
const int Nx                 = ToGrid->nx();
const int Ny                 = ToGrid->ny();

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

if ( cp_nh && cp_sh )  { Hemi = both_hemispheres;  return; }

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

if ( cp_nh && cp_sh )  { Hemi = both_hemispheres;  return; }

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


void WwmcaRegridder::get_grid()

{

Grid G = parse_vx_grid(parse_conf_regrid(Config), (Grid *) 0, (Grid *) 0);

ToGrid = new Grid(G);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for external utility functions
   //


////////////////////////////////////////////////////////////////////////


void dp_interp_min (const DataPlane & fat, DataPlane & out, int width, double fraction)

{

   //
   //  we expect that the size of the "out" dataplane has already been set
   //

int x, y, x_fat_ll, y_fat_ll;
double v;


for (x=0; x<(out.nx()); ++x)  {

   x_fat_ll = x*width;

   for (y=0; y<(out.ny()); ++y)  {

      y_fat_ll = y*width;

      v = interp_min_ll(fat, x_fat_ll, y_fat_ll, width, fraction);

      out.put(v, x, y);

   }   //  for x

}   //  for x



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void dp_interp_max (const DataPlane & fat, DataPlane & out, int width, double fraction)

{

   //
   //  we expect that the size of the "out" dataplane has already been set
   //

int x, y, x_fat_ll, y_fat_ll;
double v;


for (x=0; x<(out.nx()); ++x)  {

   x_fat_ll = x*width;

   for (y=0; y<(out.ny()); ++y)  {

      y_fat_ll = y*width;

      v = interp_max_ll(fat, x_fat_ll, y_fat_ll, width, fraction);

      out.put(v, x, y);

   }   //  for x

}   //  for x



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void dp_interp_uw_mean (const DataPlane & fat, DataPlane & out, int width, double fraction)

{

   //
   //  we expect that the size of the "out" dataplane has already been set
   //

int x, y, x_fat_ll, y_fat_ll;
double v;



for (x=0; x<(out.nx()); ++x)  {

   x_fat_ll = x*width;

   for (y=0; y<(out.ny()); ++y)  {

      y_fat_ll = y*width;

      v = interp_uw_mean_ll(fat, x_fat_ll, y_fat_ll, width, fraction);

      out.put(v, x, y);

   }   //  for x

}   //  for x



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////

