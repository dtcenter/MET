// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


static const int Nx = 500;
static const int Ny = 500;
static const int Nt =  24;

static const int x_start = 400;
static const int y_start =  50;

static const double vx_target =  -9.6;
static const double vy_target =  12.7;

static const double rect_width  = 15.0;
static const double rect_length = 30.0;

static const double angle_target = 20.0;   //  degrees


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "mtd_config_info.h"
#include "mtd_file.h"
#include "interest_calc.h"
#include "3d_att_single_array.h"
#include "mtd_txt_output.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static ConcatString output_filename = (string)"a.nc";

static LatLonData grid_data = {

   "test", 

   0.0, 90.0, 

   0.01, 0.01, 

   Ny, Nx

};


static Grid grid(grid_data);


////////////////////////////////////////////////////////////////////////


static const double C = cosd(angle_target);
static const double S = sind(angle_target);


////////////////////////////////////////////////////////////////////////


static bool inside_rect(const double u, const double v);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);


int x, y, t, k;
int vol;
MtdFloatFile raw;
MtdIntFile obj;
SingleAtt3D att;
double xcen, ycen;
double u, v;
const char model [] = "test";
int V[2];

   //
   //  make a junk raw file filled with zeroes
   //

raw.set_size(Nx, Ny, Nt);

raw.set_grid(grid);

for (x=0; x<Nx; ++x)    {

   for (y=0; y<Ny; ++y)    {

      for (t=0; t<Nt; ++t)    {

         raw.put(0.0, x, y, t);

      }

   }

}

   //
   //  make the 3D object
   //

obj.set_size(Nx, Ny, Nt);

obj.set_n_objects(1);

obj.set_grid(grid);

vol = 0;

for (t=0; t<Nt; ++t)    {

   xcen = x_start + vx_target*t;
   ycen = y_start + vy_target*t;

   for (x=0; x<Nx; ++x)    {

      u = x - xcen;

      for (y=0; y<Ny; ++y)    {

         v = y - ycen;

         k = 0;

         if ( inside_rect(u, v) )  { k = 1;  ++vol; }

         obj.put(k, x, y, t);

      }

   }

}   //  for t

V[0] = vol;

obj.set_volumes(1, V);

obj.write(output_filename.c_str());

   //
   //  get the velocity
   //

double dx, dy;
double norm;

att = calc_3d_single_atts(obj, raw, model);

dx = vx_target - att.Xvelocity;
dy = vy_target - att.Yvelocity;

norm = sqrt( dx*dx + dy*dy );

cout << "Target velocity     = (" << vx_target << ", " << vy_target << ")\n\n";
cout << "Calculated velocity = (" << att.Xvelocity << ", " << att.Yvelocity << ")\n\n";
cout << "Norm of difference  = " << norm << "\n\n";

cout << "\n\n";

norm = fabs(angle_target - att.SpatialAxisAngle);

cout << "Target spatial axis     = " << angle_target         << "\n\n";
cout << "Calculated spatial axis = " << att.SpatialAxisAngle << "\n\n";
cout << "Norm of difference      = " << norm << "\n\n";




   //
   //  done
   //

return ( 0 );

}

////////////////////////////////////////////////////////////////////////


bool inside_rect(const double u, const double v)

{

double uu, vv;

uu =  u*C + v*S;
vv = -u*S + v*C;

if ( fabs(uu) > 0.5*rect_length )  return ( false );

if ( fabs(vv) > 0.5*rect_width  )  return ( false );


return ( true );

}


////////////////////////////////////////////////////////////////////////

