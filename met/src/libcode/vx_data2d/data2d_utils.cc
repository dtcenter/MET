
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "data2d_utils.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

bool derive_wdir(const DataPlane &u2d, const DataPlane &v2d,
                 DataPlane &wdir2d) {
   int x, y;
   double u, v, wdir;
   const int nx = u2d.nx();
   const int ny = u2d.ny();

   //
   // Check that the dimensions match
   //
   if(u2d.nx() != v2d.nx() || u2d.ny() != v2d.ny()) {
      mlog << Warning << "\n\nderive_wdir() -> "
           << "the dimensions for U and V do not match: ("
           << u2d.nx() << ", " << u2d.ny() << ") != ("
           << v2d.nx() << ", " << v2d.ny() << ")\n\n";
      return(false);
   }

   //
   // Initialize
   //
   wdir2d.clear();
   wdir2d.set_size(nx, ny);

   //
   // Compute the wind direction
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         //
         // Get the U and V components for this grid point
         //
         u = u2d.get(x, y);
         v = v2d.get(x, y);

         //
         // Compute wind direction and rescale to [0, 360)
         //
         if(is_bad_data(u) || is_bad_data(v)) {
            wdir = bad_data_double;
         }
         else {
            wdir = rescale_deg(atan2d(-1.0*u, -1.0*v), 0.0, 360.0);
         }

         //
         // Store the current value
         //
         wdir2d.set(wdir, x, y);

      } // end for y
   } // end for x

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool derive_wind(const DataPlane &u2d, const DataPlane &v2d,
                 DataPlane &wind2d) {
   int x, y;
   double u, v, wind;
   const int nx = u2d.nx();
   const int ny = u2d.ny();

   //
   // Check that the dimensions match
   //
   if(u2d.nx() != v2d.nx() || u2d.ny() != v2d.ny()) {
      mlog << Warning << "\n\nderive_wind() -> "
           << "the dimensions for U and V do not match: ("
           << u2d.nx() << ", " << u2d.ny() << ") != ("
           << v2d.nx() << ", " << v2d.ny() << ")\n\n";
      return(false);
   }

   //
   // Initialize
   //
   wind2d.clear();
   wind2d.set_size(nx, ny);

   //
   // Compute the wind direction
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         //
         // Get the U and V components for this grid point
         //
         u = u2d.get(x, y);
         v = v2d.get(x, y);

         //
         // Compute wind direction and rescale to [0, 360)
         //
         if(is_bad_data(u) || is_bad_data(v)) {
            wind = bad_data_double;
         }
         else {
            wind = sqrt(u*u + v*v);
         }

         //
         // Store the current value
         //
         wind2d.set(wind, x, y);

      } // end for y
   } // end for x

   return(true);
}

////////////////////////////////////////////////////////////////////////

void rotate_wdir_grid_to_earth(const DataPlane &wdir2d, const Grid &g,
                               DataPlane &wdir2d_rot) {
   int x, y;
   double wdir_deg, wdir_deg_rot, alpha_deg;
   const int nx = wdir2d.nx();
   const int ny = wdir2d.ny();

   //
   // Initialize
   //
   wdir2d_rot.clear();
   wdir2d_rot.set_size(nx, ny);

   //
   // Rotate the wind direction
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {


         //
         // Get the wind direction for this grid point
         //
         wdir_deg = wdir2d.get(x, y);

         //
         // Compute wind direction and rescale to [0, 360)
         //
         if(is_bad_data(wdir_deg)) {
            wdir_deg_rot = bad_data_double;
         }
         else {

            // Get the rotation angle from grid to earth relative
            alpha_deg = g.rot_grid_to_earth(x, y);

            // Compute rotated wind direction
            wdir_deg_rot = rescale_deg(wdir_deg + alpha_deg, 0.0, 360.0);
         }

         //
         // Store the current value
         //
         wdir2d_rot.set(wdir_deg_rot, x, y);

      } // end for y
   } // end for x

   return;
}

////////////////////////////////////////////////////////////////////////

bool rotate_uv_grid_to_earth(const DataPlane &u2d, const DataPlane &v2d,
                             const Grid &g,
                             DataPlane &u2d_rot, DataPlane &v2d_rot) {
   int x, y;
   double u, v, alpha_deg, u_rot, v_rot;
   const int nx = u2d.nx();
   const int ny = u2d.ny();

   //
   // Check that the dimensions match
   //
   if(u2d.nx() != v2d.nx() || u2d.ny() != v2d.ny()) {
      mlog << Warning << "\n\nrotate_uv_grid_to_earth() -> "
           << "the dimensions for U and V do not match: ("
           << u2d.nx() << ", " << u2d.ny() << ") != ("
           << v2d.nx() << ", " << v2d.ny() << ")\n\n";
      return(false);
   }

   //
   // Initialize
   //
   u2d_rot.clear();
   v2d_rot.clear();
   u2d_rot.set_size(nx, ny);
   v2d_rot.set_size(nx, ny);

   //
   // Compute rotated U and V
   //
   for(x=0; x<nx; x++) {
      for(y=0; y<ny; y++) {

         //
         // Get the U and V components for this grid point
         //
         u = u2d.get(x, y);
         v = v2d.get(x, y);

         //
         // Compute rotated U and V
         //
         if(is_bad_data(u) || is_bad_data(v)) {
            u_rot = v_rot = bad_data_double;
         }
         else {

            // Get the rotation angle from grid to earth relative
            alpha_deg = g.rot_grid_to_earth(x, y);

            // Rotate U component
            u_rot = cosd(alpha_deg)*u + sind(alpha_deg)*v;

            // Rotate V component
            v_rot = -1.0*sind(alpha_deg)*u + cosd(alpha_deg)*v;
         }

         //
         // Store the current values
         //
         u2d_rot.set(u_rot, x, y);
         v2d_rot.set(v_rot, x, y);

      } // end for y
   } // end for x

   return(true);
}

////////////////////////////////////////////////////////////////////////
