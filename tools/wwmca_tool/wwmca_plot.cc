

////////////////////////////////////////////////////////////////////////


static const int gray_min         = 130;   //  these values must be between 0 and 255 inclusive
static const int gray_max         = 255;

static const double h_margin      = 40.0;
static const double v_margin      = 80.0;

static const char map_filename [] = "other/map/group1";   //  relative to DATA_DIR


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "vx_util/vx_util.h"
#include "vx_pxm/vx_pxm.h"
#include "vx_ps/vx_ps.h"
#include "vx_render/vx_render.h"
#include "vx_data_grids/vx_data_grids.h"
#include "vx_plot_util/vx_plot_util.h"

#include "afwa_file.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //


static ConcatString output_directory;


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

static void process(const char * filename);

static void make_image(const AfwaCloudPctFile &, Pxm &);

static Color value_to_color(int value);

static void grid_to_page(double x_grid, double y_grid, double & x_page, double & y_page);

static bool region_ok(const MapRegion0 &, const char hemisphere);

static void draw_map(PostScriptFile &, const char hemisphere);

static void draw_region(PostScriptFile &, const MapRegion0 &);

static void draw_latlon_grid(PostScriptFile &, const char hemisphere);

static void draw_parallel(PostScriptFile & plot, double lat);

static void draw_meridian(PostScriptFile & plot, const char hemisphere, double lon);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);


cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_outdir, "-outdir", 1);

cline.parse();

if ( cline.n() == 0 )  usage();

int j;

   //
   //  get to work
   //

for (j=0; j<(cline.n()); ++j)  {

   cout << "Plotting " << cline[j] << "\n";

   if ( (j%5) == 4 )  cout.put('\n');

   cout.flush();

   process(cline[j]);

}


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " [ -outdir path ] wwmca_cloud_pct_file_list\n\n";

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


void process(const char * filename)

{

AfwaCloudPctFile f;
ConcatString short_name;
ConcatString output_filename;
Pgm image;
RenderInfo info;
PostScriptFile plot;


if ( !(f.read(filename)) )  {

   cerr << "\n\n  " << program_name << ": unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}

if ( f.hemisphere() == 'N' )  grid = &nh;
else                          grid = &sh;

Nx = grid->nx();
Ny = grid->ny();

short_name = get_short_name(filename);

if ( output_directory.length() > 0 )  output_filename << output_directory << '/';

output_filename << short_name << ".ps";

   //
   //  make the background image
   //

make_image(f, image);

// image.write("a.pgm");

   //
   //  open the output file
   //

plot.open(output_filename);

plot.pagenumber(1);

plot.setlinecap(1);
plot.setlinejoin(1);

plot.setlinewidth(1.0);

   //
   //  render background image
   //

scale = calc_mag((double) Nx, (double) Ny, view_width, view_height);

grid_to_page(0.0, 0.0, info.x_ll, info.y_ll);

info.set_magnification(scale);

info.bw = 1;

info.add_filter(RunLengthEncode);
info.add_filter(ASCII85Encode);

render(plot, image, info);

   //
   //  set up clipping path
   //

plot.gsave();

plot.newpath();

plot.moveto(info.x_ll, info.y_ll);
plot.lineto(info.x_ll, info.y_ll);
plot.lineto(info.x_ll + scale*Nx, info.y_ll);
plot.lineto(info.x_ll + scale*Nx, info.y_ll + scale*Ny);
plot.lineto(info.x_ll, info.y_ll + scale*Ny);

plot.closepath();

plot.clip();

   //
   //  draw the map
   //

draw_map(plot, f.hemisphere());

   //
   //  unclip
   //

plot.grestore();

   //
   //  draw lat/lon grid
   //

draw_latlon_grid(plot, f.hemisphere());

   //
   //  annotate
   //

plot.choose_font(26, 12.0);

plot.write_centered_text(2, 1, 0.5*page_width, info.y_ll - 30.0, 0.5, 1.0, short_name);

   //
   //  done
   //

plot.showpage();

return;

}


////////////////////////////////////////////////////////////////////////


void make_image(const AfwaCloudPctFile & f, Pxm & image)

{

int x, y;
int value;
Color color;


image.set_size_xy(Nx, Ny);

image.all_white();


for (x=0; x<Nx; ++x)  {

   for (y=0; y<Ny; ++y)  {

      value = f(x, y);

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


bool region_ok(const MapRegion0 & r, const char hemisphere)

{

// if ( (hemisphere == 'N') && (r.lat_max() < -21.0) )  return ( false );
// 
// if ( (hemisphere == 'S') && (r.lat_min() >  21.0) )  return ( false );

return ( true );

}


////////////////////////////////////////////////////////////////////////


void draw_map(PostScriptFile & plot, const char hemisphere)

{

MapRegion0 r;
ifstream in;
ConcatString filename;

filename << DATA_DIR << '/' << map_filename;

in.open(filename);

if ( !in )  {

   cerr << "\n\n  " << program_name <<  ": unable to open map data file \"" << filename <<  "\"\n\n";

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


void draw_region(PostScriptFile & plot, const MapRegion0 & r)

{

int j;
double x_page, y_page, x_grid, y_grid;

plot.newpath();

for (j=0; j<(r.n_points()); ++j)  {

   grid->latlon_to_xy(r.lat(j), r.lon(j), x_grid, y_grid);

   grid_to_page(x_grid, y_grid, x_page, y_page);

   if ( j == 0 )  plot.moveto(x_page, y_page);
   else           plot.lineto(x_page, y_page);

}   //  for j

plot.stroke();

return;

}


////////////////////////////////////////////////////////////////////////


void draw_latlon_grid(PostScriptFile & plot, const char hemisphere)

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


void draw_parallel(PostScriptFile & plot, double lat)

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


void draw_meridian(PostScriptFile & plot, const char hemisphere, double lon)

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


