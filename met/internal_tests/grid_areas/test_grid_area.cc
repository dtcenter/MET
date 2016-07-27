

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"
#include "grid.h"
#include "nav.h"


////////////////////////////////////////////////////////////////////////


   const Grid grid(st4_ihop_data);   //  stereographic
// const Grid grid(wrf_ihop_data);   //  lambert conformal
// const Grid grid(g204_data);       //  mercator


////////////////////////////////////////////////////////////////////////


int main()

{

int x, y;
double lat_corner, lon_corner;
double lat_right, lon_right;
double lat_up, lon_up;
double area, area_approx;
double dist_bottom, dist_left;
double frac_diff, max_frac_diff;



max_frac_diff = 0.0;


for (x=0; x<(grid.nx()); ++x)  {

   for (y=0; y<(grid.ny()); ++y)  {

      grid.xy_to_latlon(x - 0.5, y - 0.5, lat_corner, lon_corner);

      grid.xy_to_latlon(x + 0.5, y - 0.5, lat_right, lon_right);

      grid.xy_to_latlon(x - 0.5, y + 0.5, lat_up, lon_up);


      dist_bottom = gc_dist(lat_corner, lon_corner, lat_right, lon_right);

      dist_left   = gc_dist(lat_corner, lon_corner, lat_up, lon_up);


      area = grid.calc_area(x, y);

      area_approx = dist_bottom*dist_left;

      frac_diff = (area - area_approx)/area;

      if ( frac_diff > max_frac_diff )  max_frac_diff = frac_diff;

   }   //  for y

}   //  for x



cout << "\n\n  max frac diff = " << max_frac_diff << "\n\n";





   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////

