// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


static const int gray_min         = 130;   //  these values must be between 0 and 255 inclusive
static const int gray_max         = 255;

static const double h_margin      = 40.0;
static const double v_margin      = 80.0;

static const char map_filename [] = "MET_BASE/map/country_major_lakes_data";


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <string.h>
#include <cmath>
#include <ctype.h>

#include "vx_log.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_pxm.h"
#include "vx_ps.h"
#include "vx_render.h"
#include "vx_grid.h"
#include "vx_plot_util.h"

#include "af_file.h"
#include "af_cp_file.h"
#include "af_pt_file.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //


static ConcatString output_directory;

static int max_minutes = 120;


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static const Grid nh(wwmca_north_data);
static const Grid sh(wwmca_south_data);

static const Grid * grid = (const Grid *) 0;

static int Nx, Ny;

static const double page_width  =  8.5*72.0;
static const double page_height = 11.0*72.0;

static const double L = h_margin;
static const double R = page_width - h_margin;

static const double B = v_margin;
static const double T = page_height - v_margin;

static const double view_width  = R - L;
static const double view_height = T - B;

static double scale;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_outdir(const StringArray &);

static void set_max_minutes(const StringArray &);

static void set_logfile(const StringArray &);

static void set_verbosity(const StringArray &);

static void process(const char * filename);

static void set_pixel_time_filename(const char *, char *);

static void make_image(const AFCloudPctFile &, const AFPixelTimeFile &, PxmBase &);

static Color value_to_color(int value);

static void grid_to_page(double x_grid, double y_grid, double & x_page, double & y_page);

static bool region_ok(const MapRegion &, const char hemisphere);

static void draw_map(PSfile &, const char hemisphere);

static void draw_region(PSfile &, const MapRegion &);

static void draw_latlon_grid(PSfile &, const char hemisphere);

static void draw_parallel(PSfile & plot, double lat);

static void draw_meridian(PSfile & plot, const char hemisphere, double lon);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);


cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_outdir, "-outdir", 1);

cline.add(set_max_minutes, "-max", 1);

cline.add(set_logfile, "-log", 1);

cline.add(set_verbosity, "-v", 1);

cline.parse();

if ( cline.n() == 0 )  usage();

int j;

   //
   //  get to work
   //

for (j=0; j<(cline.n()); ++j)  {

   mlog << Debug(1) << "Plotting " << cline[j] << "\n";

   if ( (j%5) == 4 )  mlog << Debug(1) << '\n';

   process(cline[j].c_str());

}


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cout << "\nUsage: " << program_name << "\n"
     << "\t[-outdir path]\n"
     << "\t[-max max_minutes]\n"
     << "\t[-log file]\n"
     << "\t[-v level]\n"
     << "\twwmca_cloud_pct_file_list\n\n"
     << "\twhere\t\"-outdir path\" overrides the default output "
     << "directory (.) (optional).\n"
     << "\t\t\"-max max_minutes\" overrides the default maximum "
     << "minutes of (" << max_minutes << ") (optional).\n"
     << "\t\t\"-log file\" outputs log messages to the specified "
     << "file (optional).\n"
     << "\t\t\"-v level\" overrides the default level of logging ("
     << mlog.verbosity_level() << ") (optional).\n"
     << "\t\t\"wwmca_cloud_pct_file_list\" is a list of one or "
     << "more wwmca cloud percent files to plot.\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_outdir(const StringArray & a)

{

output_directory = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_max_minutes(const StringArray & a)

{

max_minutes = atoi(a[0].c_str());

return;

}


////////////////////////////////////////////////////////////////////////


void set_logfile(const StringArray & a)

{

ConcatString filename;

filename = a[0];

mlog.open_log_file(filename);

return;

}


////////////////////////////////////////////////////////////////////////


void set_verbosity(const StringArray & a)

{

mlog.set_verbosity_level(atoi(a[0].c_str()));

return;

}


////////////////////////////////////////////////////////////////////////


void process(const char * filename)

{

AFCloudPctFile f_cp;
AFPixelTimeFile f_pt;
ConcatString short_name;
ConcatString output_filename;
char * pt_filename = (char *) 0;
Pgm image;
RenderInfo info;
PSfile plot;
double x_ll, y_ll;

if ( !(f_cp.read(filename, bad_data_char)) )  {

   mlog << Error << "\n" << program_name << ": unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}

if ( f_cp.hemisphere() == 'N' )  grid = &nh;
else                             grid = &sh;

Nx = grid->nx();
Ny = grid->ny();

short_name = get_short_name(filename);

// allocate space for the pixel time filename (make it slightly
// larger than the input filename to ensure that it is large enough)
pt_filename = new char [strlen(filename) + 10];

// create pixel time filename and read it in
set_pixel_time_filename(filename, pt_filename);

if ( !(f_pt.read(pt_filename, bad_data_char)) )  {

   mlog << Error << "\n" << program_name << ": unable to open pixel time file \"" << pt_filename << "\"\n\n";

   exit ( 1 );

}

if (pt_filename)  { delete [] pt_filename; pt_filename = (char *) 0; }

if ( output_directory.length() > 0 )  output_filename << output_directory << '/';

output_filename << short_name << ".ps";

   //
   //  make the background image
   //

make_image(f_cp, f_pt, image);

   //
   //  open the output file
   //

plot.open(output_filename.c_str());

plot.pagenumber(1);

plot.file() << " 1 setlinecap\n";
plot.file() << " 1 setlinejoin\n";

plot.setlinewidth(1.0);

   //
   //  render background image
   //

scale = calc_mag((double) Nx, (double) Ny, view_width, view_height);

grid_to_page(0.0, 0.0, x_ll, y_ll);

info.set_ll(x_ll, y_ll);

info.set_mag(scale);

info.set_color(false);

info.add_filter(RunLengthEncode);
info.add_filter(ASCII85Encode);

render(plot, image, info);

   //
   //  set up clipping path
   //

plot.gsave();

plot.newpath();

plot.moveto(info.x_ll(), info.y_ll());
plot.lineto(info.x_ll(), info.y_ll());
plot.lineto(info.x_ll() + scale*Nx, info.y_ll());
plot.lineto(info.x_ll() + scale*Nx, info.y_ll() + scale*Ny);
plot.lineto(info.x_ll(), info.y_ll() + scale*Ny);

plot.closepath();

plot.clip();

   //
   //  draw the map
   //

draw_map(plot, f_cp.hemisphere());

   //
   //  unclip
   //

plot.grestore();

   //
   //  draw lat/lon grid
   //

draw_latlon_grid(plot, f_cp.hemisphere());

   //
   //  annotate
   //

plot.choose_font(26, 12.0);

plot.write_centered_text(2, 1, 0.5*page_width, info.y_ll() - 30.0, 0.5, 1.0, short_name.c_str());

   //
   //  done
   //

plot.showpage();

return;

}


////////////////////////////////////////////////////////////////////////


// create the pixel time filename from the cloud pct filename
void set_pixel_time_filename(const char * cp_name, char * pt_name)
{
   ConcatString short_cp_name;
   static const char * pt_name_start = "WWMCA_PIXL_TIME_MEANS";
   int cp_length, short_cp_length;
   int i, j;

   // get the short name of the cloud percent file
   short_cp_name = get_short_name(cp_name);

   // get the lengths of the cloud percent filename including path and without path
   cp_length = strlen(cp_name);
   short_cp_length = short_cp_name.length();

   // copy the path to the files
   for (i = 0; i < (cp_length - short_cp_length); i++)
      pt_name[i] = cp_name[i];

   // now create the new pixel time filename which consists of the pt_name_start and the
   // rest of the cp_name after removing its start (WWMCA_TOTAL_CLOUD_PCT)
   strcpy(pt_name + i, pt_name_start);

   for (j = i + strlen(pt_name_start); j < cp_length; j++)
      pt_name[j] = cp_name[j];

   pt_name[j] = '\0';  // null terminate the string

}


////////////////////////////////////////////////////////////////////////


void make_image(const AFCloudPctFile & f_cp, const AFPixelTimeFile & f_pt, PxmBase & image)

{

int x, y;
int value;
Color color;


image.set_size_xy(Nx, Ny);

image.all_white();


for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      if ((f_pt.pixel_age_sec(x, y) / 60) < max_minutes)
         value = f_cp(x, y);
      else
         value = 0;

      color = value_to_color(value);

      image.putxy(color, x, y);

   }   //  for y

}   //  for x


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


Color value_to_color(int value)

{

Color c;
int k;

k = ((gray_max - gray_min)*value)/100 + gray_min;

c.set_gray(k);

return ( c );

}


////////////////////////////////////////////////////////////////////////


void grid_to_page(double x_grid, double y_grid, double & x_page, double & y_page)

{

x_page = scale*(x_grid - 0.5*Nx) + 0.5*(L + R);

y_page = scale*(y_grid - 0.5*Ny) + 0.5*(T + B);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool region_ok(const MapRegion & r, const char hemisphere)

{

// if ( (hemisphere == 'N') && (r.lat_max() < -21.0) )  return ( false );
//
// if ( (hemisphere == 'S') && (r.lat_min() >  21.0) )  return ( false );

return ( true );

}


////////////////////////////////////////////////////////////////////////


void draw_map(PSfile & plot, const char hemisphere)

{

MapRegion r;
ifstream in;
ConcatString filename;

filename = replace_path(map_filename);

in.open(filename.c_str());

if ( !in )  {

   mlog << Error << "\n" << program_name <<  ": unable to open map data file \"" << filename <<  "\"\n\n";

   exit ( 1 );

}

while ( in >> r )  {

   if ( region_ok(r, hemisphere) )  draw_region(plot, r);

}

   //
   //  done
   //

in.close();

return;

}


////////////////////////////////////////////////////////////////////////


void draw_region(PSfile & plot, const MapRegion & r)

{

int j;
double x_page, y_page, x_grid, y_grid;

plot.newpath();

for (j=0; j<(r.n_points); ++j)  {

   grid->latlon_to_xy(r.lat[j], r.lon[j], x_grid, y_grid);

   grid_to_page(x_grid, y_grid, x_page, y_page);

   if ( j == 0 )  plot.moveto(x_page, y_page);
   else           plot.lineto(x_page, y_page);

}   //  for j

plot.stroke();

return;

}


////////////////////////////////////////////////////////////////////////


void draw_latlon_grid(PSfile & plot, const char hemisphere)

{

int j, ms;


if ( hemisphere == 'N' )  ms =  1;
else                      ms = -1;


plot.gsave();

plot.setrgbcolor(0.3, 0.3, 1.0);

   //
   //  parallels of latitude
   //

for (j=0; j<=60; j+=30)  {

   draw_parallel(plot, ms*j);

}

   //
   //  meridians of longitude
   //

plot.choose_font(7, 10.0);

for (j=0; j<360; j+=20)  {

   draw_meridian(plot, hemisphere, j);

}



plot.grestore();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void draw_parallel(PSfile & plot, double lat)

{

int j;
double lon;
double x_grid, y_grid, x_page, y_page;
const int N = 200;

plot.newpath();

for (j=0; j<N; ++j)  {

   lon = (360.0*j)/N;

   grid->latlon_to_xy((double) lat, lon, x_grid, y_grid);

   grid_to_page(x_grid, y_grid, x_page, y_page);

   if ( j == 0 )  plot.moveto(x_page, y_page);
   else           plot.lineto(x_page, y_page);

}

plot.stroke();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void draw_meridian(PSfile & plot, const char hemisphere, double lon)

{

int j, k;
double ms, lat;
double x_grid, y_grid, x_page, y_page;
const int N = 10;
char junk[256];


if ( hemisphere == 'N' )  ms =  1.0;
else                      ms = -1.0;

plot.newpath();

for (j=0; j<=N; ++j)  {

   lat = (j*90.0)/N;

   if ( hemisphere == 'S' )  lat = -lat;

   grid->latlon_to_xy(lat, lon, x_grid, y_grid);

   grid_to_page(x_grid, y_grid, x_page, y_page);

   if ( j == 0 )  plot.moveto(x_page, y_page);
   else           plot.lineto(x_page, y_page);

}

plot.stroke();

   //
   //  label the meridian
   //

double angle;
double z, x2, y2;
double tx, ty;
const double len = 20.0;


lon += 180.0;

lon -= 360.0*floor(lon/360.0);

lon -= 180.0;

k = nint(lon);

if ( k == -180 )  k = -k;

if ( k >= 0 )  sprintf(junk, "%d W",  k);
else           sprintf(junk, "%d E", -k);


grid->latlon_to_xy(ms, lon, x_grid, y_grid);

grid_to_page(x_grid, y_grid, x2, y2);

grid->latlon_to_xy(0.0, lon, x_grid, y_grid);

grid_to_page(x_grid, y_grid, x_page, y_page);

tx = x2 - x_page;
ty = y2 - y_page;

z = sqrt( tx*tx + ty*ty );

tx /= z;
ty /= z;

angle = atan2d(ty, tx) - 90.0;

plot.gsave();

   plot.translate(x_page - len*tx, y_page - len*ty);

   plot.rotate(angle);

   plot.setgray(1.0);

   plot.setlinewidth(2.0);

   plot.write_centered_text(2, 0, 0.0, 0.0, 0.5, 0.5, junk);

   plot.setgray(0.0);

   plot.write_centered_text(2, 1, 0.0, 0.0, 0.5, 0.5, junk);

plot.grestore();


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


