// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   grid_proj_comparison.cc
//
//   Description:
//      Locate the requested grid by name, instantiate it
//      using the legacy grid code and the newer Proj-based
//      grid code, and track the maximum location difference
//      for each point in the grid.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//
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

static ConcatString program_name;

static ConcatString GridName;

////////////////////////////////////////////////////////////////////////

static void process_command_line(int, char **);
static void usage();
static void set_grid(const StringArray &);

////////////////////////////////////////////////////////////////////////

static const LambertData default_data = {

   "LambertGrid",       // name

   'N',                 // hemisphere

    60.0, 30.0,         //  scale_lat_1, scale_lat_2

    19.834, 120.051,    //  lat_pin, lot_pin

    0.0, 0.0,           //  x_pin, y_pin

    97.0,               //  lon_orient

       3.0,             //  d_km
    6371.2,             //  r_km

    1680,               //  nx
    1152,               //  ny

};

////////////////////////////////////////////////////////////////////////

int met_main(int argc, char * argv[]) {

   program_name = get_short_name(argv[0]);

   //
   // process the command line arguments
   //
   process_command_line(argc, argv);

   Grid g;
   ProjGrid p;

   //
   // instantiate the grids
   //
   if(GridName.nonempty()) {

      mlog << Debug(1) << "Reading grid named \"" << GridName << "\".\n";

      if(!find_grid_by_name(GridName.c_str(), g)) {
         mlog << Error << "\n" << program_name << " -> "
              << "Legacy Grid name \"" << GridName << "\" not found!\n\n";
         exit(1);
      }

      if(!find_grid_by_name(GridName.c_str(), p))  {
         mlog << Error << "\n" << program_name << " -> "
              << "Proj Grid name \"" << GridName << "\" not found!\n\n";
         exit(1);
      }
   }
   else {
      mlog << Debug(1) << "Reading default grid.\n";
      g.set(default_data);
      p.set(default_data);
   }

   if(mlog.verbosity_level() >= 2) {
      mlog << Debug(2) << "\nLegacy Grid Definition:\n\n";
      g.dump(cout, 1);
      mlog << Debug(2) << "\nProj Grid Definition:\n\n";
      p.dump(cout, 1);
   }

   //
   // find max difference over grid
   //
   double px, py, dx, dy, lat, lon, dist;
   double max_dist = 0.0;

   for(int gx=0; gx<(g.nx()); ++gx) {
      for(int gy=0; gy<(g.ny()); ++gy) {

         g.xy_to_latlon(gx, gy, lat, lon);
         p.latlon_to_xy(lat, lon, px, py);

         dx = (double) gx - px;
         dy = (double) gy - py;

         dist = sqrt(dx*dx + dy*dy);

         if(mlog.verbosity_level() >= 5) {
            mlog << Debug(5)
                 << "(grid_x, grid_y) = (" << gx << ", " << gy << "), "
                 << "(lat, lon) = (" << lat << ", " << lon << "), "
                 << "(proj_x, proj_y) = (" << px << ", " << py << "), "
                 << "dist = " << dist << "\n";
         }

         if(dist > max_dist) max_dist = dist;
      }
   }

   if ( !is_eq(max_dist, 0.0) ) {
      mlog << Warning << "\nMaximum distance = " << max_dist << " > 0\n\n";
   }
   else {
      mlog << Debug(1) << "Maximum distance = " << max_dist << "\n";
   }

   return(0);
}

////////////////////////////////////////////////////////////////////////

const string get_tool_name() {
   return "grid_proj_comparison";
}

////////////////////////////////////////////////////////////////////////

void process_command_line(int argc, char **argv) {
   CommandLine cline;

   //
   // parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // set the usage function
   //
   cline.set_usage(usage);

   //
   // add the options function calls
   //
   cline.add(set_grid, "-grid", 1);

   //
   // parse the command line
   //
   cline.parse();

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"

        << "\t[-grid name]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"-grid name\" overrides the default name of "
        << "grid to be analyzed (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n" << flush;

   exit(1);
}

////////////////////////////////////////////////////////////////////////

void set_grid(const StringArray &a) {
   GridName = a[0];
}

////////////////////////////////////////////////////////////////////////
