

////////////////////////////////////////////////////////////////////////


static const int anno_height             =  100;

static const int anno_roman_font = 11;
static const int anno_bold_font  = 12;


////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "cgraph.h"

#include "mode_nc_output_file.h"

#include "vx_plot_util.h"
#include "vx_config.h"
#include "configobjecttype_to_string.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static ColorTable ctable;

static MetConfig config;

static const char          outdir_name [] = "output_directory";
static const char        plotinfo_name [] = "plot_info";
static const char        plotsize_name [] = "size";
static const char     borderwidth_name [] = "border_width";
static const char         mapinfo_name [] = "map_info";
static const char        mapcolor_name [] = "line_color";
static const char backgroundcolor_name [] = "background_color";
static const char       linewidth_name [] = "line_width";
static const char        mapfiles_name [] = "map_files";
static const char       rawctable_name [] = "raw_ctable_filename";
static const char       objctable_name [] = "obj_ctable_filename";
static const char          doanno_name [] = "do_annotation";
static const char     annobgcolor_name [] = "anno_background_color";
static const char   annotextcolor_name [] = "anno_text_color";

enum PlotField {

   raw_field, 

   simple_obj_field, 

   composite_obj_field, 

   no_plot_field

};

static const Color black (  0,   0,   0);
static const Color white (255, 255, 255);
static const Color blue  (  0,   0, 255);

static const Color unmatched_color = blue;

static int Nx = 0;
static int Ny = 0;

static bool fcst_obs_set = false;

static bool do_obs = true;

static bool do_anno = true;

static bool do_data_rescale = false;

static ConcatString ctable_filename;
static ConcatString output_directory = ".";

   //
   //  default plot info
   //

static double map_linewidth = 1.0;

static int plot_size = 4;

static int border_width = 10;

static Color background_color = white;
static Color map_color        = black;
static Color anno_bg_color    = white;
static Color anno_text_color  = black;

static StringArray map_files;

static ConcatString raw_ctable_filename;
static ConcatString obj_ctable_filename;


////////////////////////////////////////////////////////////////////////


   //
   //  default values for command-line switches
   //

static PlotField plot_field = no_plot_field;

static ConcatString config_filename;   //  no default ... must be set on command line


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_config    (const StringArray &);

static void set_obs       (const StringArray &);
static void set_fcst      (const StringArray &);

static void set_raw       (const StringArray &);
static void set_simple    (const StringArray &);
static void set_composite (const StringArray &);

static void sanity_check();

static void read_config();

static void do_plot (const char * mode_nc_filename);

static void get_data_ppm (const ModeNcOutputFile &, Ppm &);

static void fill_box (const Box &, const Color &, Cgraph &);
static void clip_box (const Box &, Cgraph &);

static void draw_map (Cgraph &, const double x_ll, const double y_ll, const Grid &);

static void draw_mapfile (Cgraph &, const double x_ll, const double y_ll, const Grid &, const char *);

static void draw_region (Cgraph &, const Grid &, const double x_ll, const double y_ll, const MapRegion &);

static Color        get_dict_color  (Dictionary *, const char * id);
static int          get_dict_int    (Dictionary *, const char * id);
static ConcatString get_dict_string (Dictionary *, const char * id);
static bool         get_dict_bool   (Dictionary *, const char * id);

static void time_string(int seconds, char * out, const int len);

static void annotate(const ModeNcOutputFile &, Cgraph &, const Box &);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_config,    "-config",    1);

cline.add(set_obs,       "-obs",       0);
cline.add(set_fcst,      "-fcst",      0);

cline.add(set_raw,       "-raw",       0);
cline.add(set_simple,    "-simple",    0);
cline.add(set_composite, "-composite", 0);

cline.parse();

if ( cline.n() == 0 )  usage();

if ( config_filename.empty() )  {

   mlog << Error
        << "\n\n  " << program_name << ": no config file set!\n\n";

   exit ( 1 );

}

read_config();

sanity_check();

int j;

for (j=0; j<(cline.n()); ++j)  {

   cout << "Making plot " << (j + 1) << " of " << cline.n() << '\n' << flush;

   do_plot(cline[j]);

}




return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void read_config()

{

if ( ! config.read(config_filename) )  {

   mlog << Error
        << "\n\n  " << program_name << ": read_config() -> unable to read config file \"" 
        << config_filename << "\"\n\n";

   exit ( 1 );

}

// config.dump(cout);

int j;
const DictionaryEntry * e = 0;
const char * name = 0;
Dictionary * map_info = 0;
Dictionary * plot_info = 0;
ConcatString s;

   //
   // plot info
   //

name = plotinfo_name;

e = config.lookup(name);

if ( !e )   {

   mlog << Error
        << "\n\n  " << program_name << ": read_config() -> lookup failed for plotinfo id \"" 
        << name << "\"\n\n";

   exit ( 1 );

}

plot_info = e->dict_value();

s                   = get_dict_string(plot_info, outdir_name);
output_directory    = replace_path(s);

background_color    = get_dict_color(plot_info, backgroundcolor_name);
anno_bg_color       = get_dict_color(plot_info, annobgcolor_name);
anno_text_color     = get_dict_color(plot_info, annotextcolor_name);

plot_size           = get_dict_int(plot_info, plotsize_name);
border_width        = get_dict_int(plot_info, borderwidth_name);

s                   = get_dict_string(plot_info, rawctable_name);
raw_ctable_filename = replace_path(s);

s                   = get_dict_string(plot_info, objctable_name);
obj_ctable_filename = replace_path(s);

do_anno             = get_dict_bool(plot_info, doanno_name);

   //
   //  map info
   //

name = mapinfo_name;

e = config.lookup(name);

if ( !e )   {

   mlog << Error
        << "\n\n  " << program_name << ": read_config() -> lookup failed for mapinfo id \"" 
        << name << "\"\n\n";

   exit ( 1 );

}

map_info = e->dict_value();

map_color = get_dict_color(map_info, mapcolor_name);

   //
   //  line width
   //

name = linewidth_name;

e = map_info->lookup(name);

map_linewidth = e->d_value();

   //
   //  data files
   //

name = mapfiles_name;

e = map_info->lookup(name);

if ( !e )   {

   mlog << Error
        << "\n\n  " << program_name << ": read_config() -> lookup failed for mapfiles id \"" 
        << name << "\"\n\n";

   exit ( 1 );

}

if ( ! (e->is_array()) )  {

   mlog << Error
        << "\n\n  " << program_name << ": read_config() -> id \"" 
        << name << "\" is not an array!\n\n";

   exit ( 1 );

}

Dictionary & a = *(e->array_value());

for (j=0; j<(a.n_entries()); ++j)  {

   e = a[j];

   if ( e->type() != StringType )  {

      mlog << Error
           << "\n\n  " << program_name << ": read_config() -> entry " << j << " of \"" 
           << name << "\" is not a string!\n\n";

      // mlog << Error << "\n\n  " << configobjecttype_to_string(e->type()) << "\n\n";

      exit ( 1 );

   }

   s = *(e->string_value());

   map_files.add(replace_path(s));

}

// map_files.dump(cout);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_plot(const char * mode_nc_filename)

{

ModeNcOutputFile mode_in;
ConcatString output_filename;
Cgraph plot;
Ppm image;
Box whole_box, map_box, anno_box;


output_filename << output_directory << '/' << get_short_name(mode_nc_filename);

output_filename.chomp(".nc");

switch ( plot_field )  {

   case raw_field:
      output_filename << "_raw.png";
      break;

   case simple_obj_field:
      output_filename << "_simple.png";
      break;

   case composite_obj_field:
      output_filename << "_comp.png";
      break;

      default:
      mlog << Error << "\n\n  " << program_name << ": do_plot() -> bad field selected\n\n";
      exit ( 1 );
      break;

}   //  switch


   //
   //  open the mode file
   //

if ( ! mode_in.open(mode_nc_filename) )  {

   mlog << Error
        << "\n\n  " << program_name << ": unable to open MODE netcdf file \""
        << mode_nc_filename << "\"\n\n";

   exit ( 1 );

}

// mode_in.dump(cout);

   //
   //  read the colortable
   //

if ( ! ctable.read(ctable_filename) )  {

   mlog << Error
        << "\n\n  " << program_name << ": unable to open colortable file \""
        << ctable_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  get the grid size
   //

Nx = plot_size*(mode_in.nx());
Ny = plot_size*(mode_in.ny());

if ( do_anno )  {

   whole_box.set_llwh(0.0, 0.0, Nx + 2*border_width, Ny + anno_height + border_width);

   map_box.set_llwh(border_width, anno_height, Nx, Ny);

   anno_box.set_llwh(0.0, 0.0, Nx + 2*border_width, anno_height);

} else {

   map_box.set_llwh(0.0, 0.0, Nx, Ny);

   whole_box = map_box;

}

   //
   //  open the plot
   //

plot.open(output_filename, whole_box.width(), whole_box.height());

if ( do_anno )   fill_box(whole_box, anno_bg_color, plot);

   //
   //  get a ppm of the data field
   //

get_data_ppm(mode_in, image);

plot.import(image, map_box.left(), map_box.bottom(), 0.0, 0.0, 1.0);

   //
   //  draw the map
   //

if ( do_anno )  {   //  set up clipping path, if needed

   plot.gsave();

   clip_box(map_box, plot);

}

draw_map(plot, map_box.left(), map_box.bottom(), mode_in.grid());

if ( do_anno )  plot.grestore();

   //
   //  do the annotation, if needed
   //

if ( do_anno )  annotate(mode_in, plot, anno_box);

   //
   //  done
   //

plot.showpage();

mode_in.close();

return;

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << ": -simple|-composite|-raw  -obs|-fcst -config path mode_nc_file_list\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_config (const StringArray & a)

{

config_filename = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_raw(const StringArray &)

{

plot_field = raw_field;

do_data_rescale = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_obs(const StringArray &)

{

do_obs = true;

fcst_obs_set = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_fcst(const StringArray &)

{

do_obs = false;

fcst_obs_set = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_simple(const StringArray &)

{

plot_field = simple_obj_field;

return;

}


////////////////////////////////////////////////////////////////////////


void set_composite(const StringArray &)

{

plot_field = composite_obj_field;

return;

}


////////////////////////////////////////////////////////////////////////


void sanity_check()

{

if ( !fcst_obs_set )  {

   mlog << Error
        << "\n\n  " << program_name << ": fcst/obs not set!\n\n";

   exit ( 1 );

}

if ( plot_field == no_plot_field )  {

   mlog << Error
        << "\n\n  " << program_name << ": plot field not set!\n\n";

   exit ( 1 );

}

if ( raw_ctable_filename.empty() )  {

   mlog << Error
        << "\n\n  " << program_name << ": no raw colortable file set!\n\n";

   exit ( 1 );

}

if ( obj_ctable_filename.empty() )  {

   mlog << Error
        << "\n\n  " << program_name << ": no object colortable file set!\n\n";

   exit ( 1 );

}

if ( border_width < 0 )  border_width = 0;

if ( ! do_anno )  border_width = 0;

if ( plot_size < 1 )  plot_size = 1;

if ( plot_field == raw_field )  ctable_filename = raw_ctable_filename;
else                            ctable_filename = obj_ctable_filename;

return;

}


////////////////////////////////////////////////////////////////////////


void get_data_ppm(const ModeNcOutputFile & mode_in, Ppm & image)

{

int k;
int x, y, mx, my;
double value, M, B;
double min_value, max_value;
Color color;


if ( do_data_rescale )  {

   mode_in.get_fcst_raw_range(min_value, max_value);

   M = 1.0/(max_value - min_value);

   B = -M*min_value;

}


image.set_size_xy(Nx, Ny);

for (x=0; x<(image.nx()); ++x)  {

   mx = x/plot_size;

   for (y=0; y<(image.ny()); ++y)  {

      my = y/plot_size;

      // image.putxy(background_color, x, y);

      if ( plot_field == raw_field )  {

         if ( do_obs )  value = mode_in.obs_raw  (mx, my);
         else           value = mode_in.fcst_raw (mx, my);

         if ( do_data_rescale && (value > -9000.0) )  value = M*value + B;

         color = ctable.interp(value);

      } else {   //  object

         if ( plot_field == composite_obj_field )  {

            if ( do_obs )  k = mode_in.obs_comp_id  (mx, my);
            else           k = mode_in.fcst_comp_id (mx, my);

         } else {   //  simple

            if ( do_obs )  k = mode_in.obs_obj_id  (mx, my);
            else           k = mode_in.fcst_obj_id (mx, my);

         }

         value = (double) k;

         if ( k == -1 )  color = unmatched_color;
         else            color = ctable.nearest(value);

      }

      if ( color == white )  color = background_color;

      image.putxy(color, x, y);

   }

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void fill_box(const Box & b, const Color & c, Cgraph & plot)

{

plot.gsave();

   plot.set_color(c);

   plot.newpath();

   plot.moveto(b.left(),  b.bottom());
   plot.lineto(b.right(), b.bottom());
   plot.lineto(b.right(), b.top());
   plot.lineto(b.left(),  b.top());

   plot.closepath();

   plot.fill();

plot.grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void clip_box(const Box & b, Cgraph & plot)

{

plot.newpath();

plot.moveto(b.left(),  b.bottom());
plot.lineto(b.right(), b.bottom());
plot.lineto(b.right(), b.top());
plot.lineto(b.left(),  b.top());

plot.closepath();

plot.clip();

return;

}


////////////////////////////////////////////////////////////////////////


void draw_map(Cgraph & plot, const double x_ll, const double y_ll, const Grid & grid)

{

int j;


for (j=0; j<(map_files.n_elements()); ++j)  {

   draw_mapfile(plot, x_ll, y_ll, grid, map_files[j]);

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void draw_mapfile(Cgraph & plot, const double x_ll, const double y_ll, const Grid & grid, const char * map_filename)

{

ifstream in;
MapRegion r;


in.open(map_filename);

if ( !in )  {

   mlog << Error
        << "\n\n  " << program_name << ": draw_mapfile() -> unable to open map data file \""
        << map_filename << "\"\n\n";

   exit ( 1 );

}

plot.gsave();

   plot.setlinewidth(map_linewidth);

   plot.set_color(map_color);

   while ( in >> r )  {

      draw_region(plot, grid, x_ll, y_ll, r);

   }

plot.grestore();



   //
   //  done
   //

in.close();

return;

}


////////////////////////////////////////////////////////////////////////


void draw_region(Cgraph & plot, const Grid & grid, const double x_ll, const double y_ll, const MapRegion & r)

{

int j;
double x_grid, y_grid;
double x_page, y_page;

plot.newpath();

for (j=0; j<(r.n_points); ++j)  {

   grid.latlon_to_xy(r.lat[j], r.lon[j], x_grid, y_grid);

   x_page = x_grid*plot_size;
   y_page = y_grid*plot_size;

   x_page += x_ll;
   y_page += y_ll;

   if ( j == 0 )  plot.moveto(x_page, y_page);
   else           plot.lineto(x_page, y_page);

}

plot.stroke();

return;

}


////////////////////////////////////////////////////////////////////////


ConcatString get_dict_string (Dictionary * dict, const char * id)

{

ConcatString s;
const DictionaryEntry * e = 0;


e = dict->lookup(id);

if ( !e )  {

   mlog << Error
        << "\n\n  " << program_name << ": get_dict_string() -> lookup failed for \""
        << id << "\"\n\n";

   exit ( 1 );

}

if ( e->type() != StringType )  {

   mlog << Error
        << "\n\n  " << program_name << ": get_dict_string() -> entry \""
        << id << "\" is not a string\n\n";

   exit ( 1 );

}

s = *(e->string_value());

return ( s );

}


////////////////////////////////////////////////////////////////////////


bool get_dict_bool(Dictionary * dict, const char * id)

{

bool tf = false;
const DictionaryEntry * e = 0;


e = dict->lookup(id);

if ( !e )  {

   mlog << Error
        << "\n\n  " << program_name << ": get_dict_bool() -> lookup failed for \""
        << id << "\"\n\n";

   exit ( 1 );

}

tf = e->b_value();

return ( tf );

}


////////////////////////////////////////////////////////////////////////


int get_dict_int(Dictionary * dict, const char * id)

{

int k = 0;
const DictionaryEntry * e = 0;


e = dict->lookup(id);

if ( !e )  {

   mlog << Error
        << "\n\n  " << program_name << ": get_dict_int() -> lookup failed for \""
        << id << "\"\n\n";

   exit ( 1 );

}

k = e->i_value();

return ( k );

}


////////////////////////////////////////////////////////////////////////


Color get_dict_color(Dictionary * dict, const char * id)

{

int j;
int rgb[3];
Color c;
const DictionaryEntry * e = 0;


e = dict->lookup(id);

if ( !e )  {

   mlog << Error
        << "\n\n  " << program_name << ": get_dict_color() -> lookup failed for color \""
        << id << "\n\n";

   exit ( 1 );

}

const Dictionary & array = *(e->array_value());

for (j=0; j<3; ++j)  {

   e = array[j];

   rgb[j] = e->i_value();

}

c.set_rgb(rgb[0], rgb[1], rgb[2]);

return ( c );

}


////////////////////////////////////////////////////////////////////////


void time_string(int seconds, char * out, const int len)

{

int h, m, s;

h = seconds/3600;
m = (seconds%3600)/60;
s = seconds%60;

     if ( (m == 0) && (s == 0) )  snprintf(out, len, "%02dh",          h);
else if ( m == 0 )                snprintf(out, len, "%02d:%02d",      h, m);
else                              snprintf(out, len, "%02d:%02d:%02d", h, m, s);

return;

}


////////////////////////////////////////////////////////////////////////


void annotate(const ModeNcOutputFile & mode_in, Cgraph & plot, const Box & anno_box)

{

ConcatString title;
ConcatString fcst_obs, raw_obj;
ConcatString s;
const double title_font_size = 30.0;
char junk[256], ts[256];
int month, day, year, hour, minute, second;
int lead_seconds;
double y;
double htab1, htab2;


// mode_in.dump(cout);

htab1 = border_width + 10.0;

htab2 = htab1 + 60.0;

   //
   //  title
   //

if ( do_obs )  fcst_obs = "Observed";
else           fcst_obs = "Forecast";

switch ( plot_field )  {

   case raw_field:            raw_obj = "Raw";               break;
   case simple_obj_field:     raw_obj = "Simple Object";     break;
   case composite_obj_field:  raw_obj = "Composite Object";  break;

   default:
      mlog << Error << "\n\n  " << program_name << ": annotate() -> bad plot field\n\n";
      exit ( 1 );
      break;

}

plot.set_color(anno_text_color);

title << cs_erase
      << fcst_obs << ' '
      << raw_obj  << ' '
      << "Field";

plot.choose_font(anno_bold_font, title_font_size);

plot.write_centered_text(1, 1, 0.5*(plot.page_width()), anno_height - title_font_size - 10.0, 0.5, 0.0, title);


plot.choose_font(anno_roman_font, 20.0);

plot.write_centered_text(1, 1, 0.5*(plot.page_width()), 25.0, 0.5, 0.0, mode_in.short_filename());


   //
   // lead time
   //

y = anno_height + 20.0;

plot.choose_font(anno_bold_font, 20.0);
plot.set_color(black);
plot.write_centered_text(1, 1, htab1, y, 0.0, 0.0, "Lead");

plot.choose_font(anno_roman_font, 20.0);
plot.set_color(anno_text_color);

lead_seconds = (int) (mode_in.valid_time() - mode_in.init_time());

time_string(lead_seconds, ts, sizeof(ts));

plot.write_centered_text(1, 1, htab2, y, 0.0, 0.0, ts);

   //
   //  valid time
   //

y += 30.0;

plot.choose_font(anno_bold_font, 20.0);
plot.set_color(black);
plot.write_centered_text(1, 1, htab1, y, 0.0, 0.0, "Valid");

plot.choose_font(anno_roman_font, 20.0);
plot.set_color(anno_text_color);

unix_to_mdyhms(mode_in.valid_time(), month, day, year, hour, minute, second);

time_string((int) (mode_in.valid_time()%86400), ts, sizeof(ts));

snprintf(junk, sizeof(junk), "%s %d, %d  %s", short_month_name[month], day, year, ts);

plot.write_centered_text(1, 1, htab2, y, 0.0, 0.0, junk);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


