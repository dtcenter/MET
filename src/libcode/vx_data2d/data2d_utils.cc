// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "data2d_utils.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_log.h"

using namespace std;

////////////////////////////////////////////////////////////////////////

static bool derive_wind_speed_and_direction(
               const DataPlane &u2d, const DataPlane &v2d,
               bool want_wspd,
               DataPlane &dp);

static bool derive_u_and_v_wind(
               const DataPlane &spd, const DataPlane &dir,
               bool want_uwnd,
               DataPlane &dp);

////////////////////////////////////////////////////////////////////////

bool build_grid_by_grid_string(
        const char *grid_str, Grid &grid,
        const char *caller_name, bool do_warning) {
   bool status = false;

   if (nullptr != grid_str && m_strlen(grid_str) > 0) {
      // Parse as a white-space separated string

      StringArray sa;
      sa.parse_wsss(grid_str);

      // Search for a named grid
      if (sa.n() == 1 && find_grid_by_name(sa[0].c_str(), grid)) {
         status = true;
         mlog << Debug(3) << "Use the grid named \"" << grid_str << "\".\n";
      }
      // Parse grid definition
      else if (sa.n() > 1 && parse_grid_def(sa, grid)) {
         status = true;
         mlog << Debug(3) << "Use the grid defined by string \""
              << grid_str << "\".\n";
      }
      else if (do_warning) {
         mlog << Warning << "\nbuild_grid_by_grid_string() by " << caller_name
              << " unsupported " << conf_key_set_attr_grid
              << " definition string (" << grid_str
              << ")!\n\n";
      }
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

bool build_grid_by_grid_string(
        const ConcatString &grid_str, Grid &grid,
        const char *caller_name, bool do_warning) {
   bool status = false;

   if(grid_str.nonempty()) {
      status = build_grid_by_grid_string(grid_str.c_str(), grid,
                                           caller_name, do_warning);
   }

   return status;
}

////////////////////////////////////////////////////////////////////////

bool derive_wind_speed(
        const DataPlane &uwnd, const DataPlane &vwnd,
        DataPlane &wspd) {

   mlog << Debug(3)
        << "Deriving wind speed from U and V wind components.\n";

   return derive_wind_speed_and_direction(uwnd, vwnd, true, wspd);
}

////////////////////////////////////////////////////////////////////////

bool derive_wind_direction(
        const DataPlane &uwnd, const DataPlane &vwnd,
        DataPlane &wdir) {

   mlog << Debug(3)
        << "Deriving wind direction from U and V wind components.\n";

   return derive_wind_speed_and_direction(uwnd, vwnd, false, wdir);
}

////////////////////////////////////////////////////////////////////////

static bool derive_wind_speed_and_direction(
               const DataPlane &uwnd, const DataPlane &vwnd,
               bool want_wspd,
               DataPlane &dp) {

   //
   // Check that the dimensions match
   //
   if(uwnd.nx() != vwnd.nx() || uwnd.ny() != vwnd.ny()) {
      mlog << Warning << "\nderive_wind_speed_and_direction() -> "
           << "the dimensions for U and V do not match: ("
           << uwnd.nx() << ", " << uwnd.ny() << ") != ("
           << vwnd.nx() << ", " << vwnd.ny() << ")\n\n";
      return false;
   }

   //
   // Initialize output
   //
   dp = uwnd;
   dp.set_constant(bad_data_double);

   const int nx = uwnd.nx();
   const int ny = uwnd.ny();

#pragma omp parallel default(shared) \
   shared(uwnd, vwnd, dp)
   {

#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {

            //
            // Get the U and V components for this grid point
            //
            double u = uwnd.get(x, y);
            double v = vwnd.get(x, y);

            double der_v = bad_data_double;

            //
            // Derive value
            //
            if(!is_bad_data(u) && !is_bad_data(v)) {
               if(want_wspd) {
                  der_v = sqrt(u*u + v*v);
               }
               else {
                  der_v = rescale_deg(atan2d(-1.0*u, -1.0*v), 0.0, 360.0);
               }
            }

            //
            // Store the derived value
            //
            dp.set(der_v, x, y);

         } // end for y
      } // end for x
   } // End omp parallel

   return true;
}

////////////////////////////////////////////////////////////////////////

bool derive_kinetic_energy(
        const DataPlane &uwnd, const DataPlane &vwnd,
        DataPlane &keng) {

   mlog << Debug(3)
        << "Deriving kinetic energy from U and V wind components.\n";

   //
   // Check that the dimensions match
   //
   if(uwnd.nx() != vwnd.nx() || uwnd.ny() != vwnd.ny()) {
      mlog << Warning << "\nderive_kinetic_energy() -> "
           << "the dimensions for U and V do not match: ("
           << uwnd.nx() << ", " << uwnd.ny() << ") != ("
           << vwnd.nx() << ", " << vwnd.ny() << ")\n\n";
      return false;
   }

   //
   // Initialize output
   //
   keng = uwnd;
   keng.set_constant(bad_data_double);

   const int nx = uwnd.nx();
   const int ny = uwnd.ny();

#pragma omp parallel default(shared) \
   shared(uwnd, vwnd, keng)
   {

#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {

            //
            // Get the U and V components for this grid point
            //
            double u = uwnd.get(x, y);
            double v = vwnd.get(x, y);

            double der_v = bad_data_double;

            //
            // Derive value
            //
            if(!is_bad_data(u) && !is_bad_data(v)) {
               der_v = (u*u + v*v)/2.0;
               keng.set(der_v, x, y);
            }

         } // end for y
      } // end for x
   } // End omp parallel

   return true;
}

////////////////////////////////////////////////////////////////////////

bool derive_u_wind(
        const DataPlane &wspd, const DataPlane &wdir,
        DataPlane &uwnd) {

   mlog << Debug(3)
        << "Deriving U wind from wind speed and direction.\n";

   return derive_u_and_v_wind(wspd, wdir, true, uwnd);
}

////////////////////////////////////////////////////////////////////////

bool derive_v_wind(
        const DataPlane &wspd, const DataPlane &wdir,
        DataPlane &vwnd) {

   mlog << Debug(3)
        << "Deriving V wind from wind speed and direction.\n";

   return derive_u_and_v_wind(wspd, wdir, false, vwnd);
}

////////////////////////////////////////////////////////////////////////

static bool derive_u_and_v_wind(
               const DataPlane &wspd, const DataPlane &wdir,
               bool want_uwnd,
               DataPlane &dp) {

   //
   // Check that the dimensions match
   //
   if(wspd.nx() != wdir.nx() || wspd.ny() != wdir.ny()) {
      mlog << Warning << "\nderive_u_and_v_wind() -> "
           << "the dimensions for wind speed and direction do not match: ("
           << wspd.nx() << ", " << wspd.ny() << ") != ("
           << wdir.nx() << ", " << wdir.ny() << ")\n\n";
      return false;
   }

   //
   // Initialize output
   //
   dp = wspd;
   dp.set_constant(bad_data_double);

   const int nx = wspd.nx();
   const int ny = wspd.ny();

#pragma omp parallel default(shared) \
   shared(wspd, wdir, dp)
   {

#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {

            //
            // Get the wind speed and direction for this grid point
            //
            double s = wspd.get(x, y);
            double d = wdir.get(x, y);

            double der_v = bad_data_double;

            //
            // Derive value
            //
            if(!is_bad_data(s) && !is_bad_data(d)) {
               if(want_uwnd) der_v = s * cosd(270.0 - d);
               else          der_v = s * sind(270.0 - d);
            }

            //
            // Store the derived value
            //
            dp.set(der_v, x, y);

         } // end for y
      } // end for x
   } // End omp parallel

   return true;
}

////////////////////////////////////////////////////////////////////////

void rotate_wind_direction_grid_to_earth(
        const DataPlane &wdir2d, const Grid &g,
        DataPlane &wdir2d_rot) {
   const int nx = wdir2d.nx();
   const int ny = wdir2d.ny();

   //
   // Initialize by setting to u2d
   //
   wdir2d_rot = wdir2d;
   wdir2d_rot.set_constant(bad_data_double);

#pragma omp parallel default(shared) \
   shared(wdir2d, g, wdir2d_rot)
   {

      //
      // Rotate the wind direction
      //
#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {

            //
            // Get the wind direction for this grid point
            //
            double wdir_deg = wdir2d.get(x, y);

            double wdir_deg_rot = bad_data_double;

            //
            // Compute wind direction and rescale to [0, 360)
            //
            if(!is_bad_data(wdir_deg)) {

               // Get the rotation angle from grid to earth relative
               double alpha_deg = g.rot_grid_to_earth(x, y);

               // Compute rotated wind direction
               wdir_deg_rot = rescale_deg(wdir_deg + alpha_deg, 0.0, 360.0);
            }

            //
            // Store the current value
            //
            wdir2d_rot.set(wdir_deg_rot, x, y);

         } // end for y
      } // end for x
   } // End omp parallel

   return;
}

////////////////////////////////////////////////////////////////////////

bool rotate_uv_grid_to_earth(
        const DataPlane &u2d, const DataPlane &v2d,
        const Grid &g,
        DataPlane &u2d_rot, DataPlane &v2d_rot) {
   const int nx = u2d.nx();
   const int ny = u2d.ny();

   //
   // Check that the dimensions match
   //
   if(u2d.nx() != v2d.nx() || u2d.ny() != v2d.ny()) {
      mlog << Warning << "\nrotate_uv_grid_to_earth() -> "
           << "the dimensions for U and V do not match: ("
           << u2d.nx() << ", " << u2d.ny() << ") != ("
           << v2d.nx() << ", " << v2d.ny() << ")\n\n";
      return false;
   }

   //
   // Initialize
   //
   u2d_rot = u2d;
   v2d_rot = v2d;
   u2d_rot.set_constant(bad_data_double);
   v2d_rot.set_constant(bad_data_double);

#pragma omp parallel default(shared) \
   shared(u2d, v2d, g, u2d_rot, v2d_rot)
   {

      //
      // Rotate the wind direction
      //
#pragma omp for schedule(static) \
                collapse(2)
      for(int x=0; x<nx; x++) {
         for(int y=0; y<ny; y++) {

            //
            // Get the U and V components for this grid point
            //
            double u = u2d.get(x, y);
            double v = v2d.get(x, y);

            double u_rot = bad_data_double;
            double v_rot = bad_data_double;

            //
            // Compute rotated U and V
            //
            if(!is_bad_data(u) && !is_bad_data(v)) {

               // Get the rotation angle from grid to earth relative
               double alpha_deg = g.rot_grid_to_earth(x, y);

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
   } // End omp parallel

   return true;
}

////////////////////////////////////////////////////////////////////////

void set_attrs(const VarInfo *info, DataPlane &dp) {

   if(!info) return;

   //
   // Update attributes, if requested
   //

   // init_time
   if(info->init_attr() != (unixtime) 0) {
      mlog << Debug(3) << "Resetting initialization time from "
           << unix_to_yyyymmdd_hhmmss(dp.init()) << " to "
           << unix_to_yyyymmdd_hhmmss(info->init_attr()) << ".\n";
      dp.set_init(info->init_attr());
   }

   // valid_time
   if(info->valid_attr() != (unixtime) 0) {
      mlog << Debug(3) << "Resetting valid time from "
           << unix_to_yyyymmdd_hhmmss(dp.valid()) << " to "
           << unix_to_yyyymmdd_hhmmss(info->valid_attr()) << ".\n";
      dp.set_valid(info->valid_attr());
   }

   // lead_time
   if(!is_bad_data(info->lead_attr())) {
      mlog << Debug(3) << "Resetting lead time from "
           << sec_to_hhmmss(dp.lead()) << " to "
           << sec_to_hhmmss(info->lead_attr()) << ".\n";
      dp.set_lead(info->lead_attr());
   }

   // accum_time
   if(!is_bad_data(info->accum_attr())) {
      mlog << Debug(3) << "Resetting accumulation interval from "
           << sec_to_hhmmss(dp.accum()) << " to "
           << sec_to_hhmmss(info->accum_attr()) << ".\n";
      dp.set_accum(info->accum_attr());
   }

   return;
}

////////////////////////////////////////////////////////////////////////
