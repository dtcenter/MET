

////////////////////////////////////////////////////////////////////////


static const char  input_filename [] = "/d1/bullock/otherlibs/met/METv4.1/out/mode/mode_240000L_20050808_000000V_240000A_obj.nc";

static const char ctable_filename [] = "/d1/bullock/otherlibs/met/METv4.1/data/colortables/met_default.ctable";

static const char output_filename [] = "a.png";

static const int Nx                  = 1000;
static const int Ny                  =  800;

static const int anno_height         =  150;

static const bool do_rescale         = true;


////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "cgraph.h"

#include "mode_nc_file.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static ColorTable ctable;

static const Color black (0, 0, 0);


////////////////////////////////////////////////////////////////////////


static void get_data_ppm(const ModeNcFile &, Ppm &);

static void fill_box(const Box &, const Color &, Cgraph &);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

ModeNcFile mode_in;
Cgraph plot;
Ppm image;
Box top_box, bot_box, image_box;
double mag;


top_box.set_llwh(0.0, anno_height, Nx, Ny - anno_height);

bot_box.set_llwh(0.0, 0.0, Nx, anno_height);

   //
   //  open the mode file
   //

if ( ! mode_in.open(input_filename) )  {

   mlog << Error
        << "\n\n  " << program_name << ": unable to open MODE netcdf file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

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
   //  open the plot
   //

plot.open(output_filename, Nx, Ny);

   //
   //  get a ppm of the data field
   //

get_data_ppm(mode_in, image);

mag = calc_mag(image.nx(), image.ny(), top_box.width(), top_box.height());

image_box.set_llwh(top_box.left()   + 0.5*(top_box.width())  - 0.5*mag*(image.nx()),
                   top_box.bottom() + 0.5*(top_box.height()) - 0.5*mag*(image.ny()),
                   mag*(image.nx()), mag*(image.ny()));

fill_box(top_box, black, plot);

plot.import(image, image_box.left(), image_box.bottom(), 0.0, 0.0, mag);













   //
   //  done
   //

plot.showpage();

mode_in.close();

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void get_data_ppm(const ModeNcFile & mode_in, Ppm & image)

{

int x, y;
double value, M, B;
double min_value, max_value;
Color color;



if ( do_rescale )  {

   mode_in.get_fcst_raw_range(min_value, max_value);

   M = 1.0/(max_value - min_value);

   B = -M*min_value;

}


image.set_size_xy(mode_in.nx(), mode_in.ny());

for (x=0; x<(mode_in.nx()); ++x)  {

   for (y=0; y<(mode_in.ny()); ++y)  {

      value = mode_in.fcst_raw(x, y);

      if ( do_rescale && (value > -9000.0) )  value = M*value + B;

      color = ctable.interp(value);

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


