

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"
#include "vx_math.h"
#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


static const LambertData data = {

   "adam_clark",        // name

   'N',                 // hemisphere

    60.0, 30.0,         //  scale_lat_1, scale_lat_2

    19.834, 120.051,    //  lat_pin, lot_pin

    0.0, 0.0,           //  x_pin, y_pin
    // 5.0, 10.0,           //  x_pin, y_pin

    97.0,               //  lon_orient

       3.0,             //  d_km
    6371.2,             //  r_km

    1680,               //  nx
    1152,               //  ny

};


////////////////////////////////////////////////////////////////////////


int main()

{

const Grid g(data);
ProjGrid p;
Affine aff;
ConcatString proj_params;
double xx, yy;
char junk[256];


cout << "\n\n";

   //
   //  create the PROJ part of the transformation
   //

   //
   //  Note: PROJ lambert params explained at:
   //
   //     https://proj.org/en/9.3/operations/projections/lcc.html#lambert-conformal-conic
   //

   //   specify lambert conformal projection

proj_params = "+proj=lcc";   //  lcc means lambert conformal conic

   //   radius of earth

sprintf(junk, "+R=%.3f", data.r_km);   //  earth radius

proj_params << ' ' << junk;

   //  the two secant latitudes for the lambert projection

sprintf(junk, "+lat_1=%.3f +lat_2=%.3f", data.scale_lat_1, data.scale_lat_2);

proj_params << ' ' << junk;

   //  orientation longitude
   //    note: PROJ calls it "Longitude of projection center"

sprintf(junk, "+lon_0=%.3f", -(data.lon_orient));   //  note minus sign

proj_params << ' ' << junk;

   //  false easting and northing
   //     (on second thought, we'll let the Affine class handle this)

   //  ok

cout << "proj_params = \"" << proj_params << "\"\n\n" << flush;

p.set_proj(proj_params.c_str());

   //
   //  create the affine part of the transformation
   //

const double s = 1.0/(data.d_km);   //  scale factor

aff.set_mb(s, 0.0, 0.0, s, 0.0, 0.0);

p.set_affine(aff);


p.latlon_to_xy(data.lat_pin, data.lon_pin, xx, yy);

// Commented out since Affine::set_b() is not defined
// aff.set_b(data.x_pin - xx, data.y_pin - yy);

p.set_affine(aff);   //  again

   //
   //  misc
   //

p.set_size(data.nx, data.ny);

p.set_name(data.name);

   //
   //  find max error over grid
   //

int i, j;
double er, max_er;
double lat, lon;
double px, py;


max_er = 0.0;

for (i=0; i<(g.nx()); ++i)  {

   for (j=0; j<(g.ny()); ++j)  {

      g.xy_to_latlon(i, j, lat, lon);

      p.latlon_to_xy(lat, lon, px, py);

      er = fabs(i - px) + fabs(j - py);

      if ( er > max_er )  max_er = er;

   }

}


cout << "max error = " << max_er << "\n\n";



   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////





