// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2010
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////
//
//   Filename:   plot_point_obs.cc
//
//   Description:
//      This tool creates a postscript plot marking the locations of
//      point observations contained in the input NetCDF point
//      observation file.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    06-10-10  Halley Gotway   New
//   001    10/19/11  Holmes          Added use of command line class to
//                                    parse the command line arguments.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "netcdf.hh"

#include "vx_met_util.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "grid.h"
#include "vx_math.h"
#include "vx_color.h"
#include "vx_ps.h"
#include "vx_pxm.h"
#include "vx_render.h"
#include "vx_plot_util.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char  *program_name = "plot_point_obs";
static const Color  c_map(25, 25, 25);
static const double l_width = 0.5;

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static char        met_data_dir[PATH_MAX];
static Grid        grid("G004");
static BoundingBox ll_bb, xy_bb;
static StringArray ityp;
static IntArray    ivar;
static double      lat_ll, lon_ll, lat_ur, lon_ur;


///////////////////////////////////////////////////////////////////////////////

static void draw_border(PSfile &, BoundingBox &);
static void draw_map(PSfile &, BoundingBox &);
static void usage();
static void set_grib_code(const StringArray &);
static void set_msg_type(const StringArray &);
static void set_data_dir(const StringArray &);
static void set_dim(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   CommandLine cline;
   ConcatString nc_file, ps_file;
   char hdr_typ_str[max_str_len];
   float *hdr_arr, *obs_arr;
   int i, h, v, count;
   PSfile plot;
   BoundingBox dim;
   double lat, lon, page_x, page_y;
   IntArray ihdr;

   //
   // check for zero arguments
   //
   if (argc == 1)
      usage();

   //
   // Initialize the x/y bounding box to the whole grid
   //
   xy_bb.x_ll   = 0;
   xy_bb.y_ll   = 0;
   xy_bb.x_ur   = grid.nx();
   xy_bb.y_ur   = grid.ny();
   xy_bb.width  = xy_bb.x_ur - xy_bb.x_ll;
   xy_bb.height = xy_bb.y_ur - xy_bb.y_ll;

   //
   // Initialize the data directory using MET_BASE
   //
   sprintf(met_data_dir, "%s/data", MET_BASE);

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
   cline.add(set_grib_code, "-gc", 1);
   cline.add(set_msg_type, "-msg_typ", 1);
   cline.add(set_data_dir, "-data_dir", 1);
   cline.add(set_dim, "-dim", 4);

   //
   // parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be two arguments left; the
   // netCDF point observation filename and the output PostScript
   // filename.
   //
   if (cline.n() != 2)
      usage();

   //
   // Store the input arguments
   //
   nc_file = cline[0];
   ps_file = cline[1];

   //
   // Declare NetCDF file, dimensions, and variables
   //
   NcFile *f_in       = (NcFile *) 0;

   NcDim *hdr_arr_dim = (NcDim *) 0;
   NcDim *obs_arr_dim = (NcDim *) 0;
   NcDim *nhdr_dim    = (NcDim *) 0;
   NcDim *nobs_dim    = (NcDim *) 0;
   NcDim *strl_dim    = (NcDim *) 0;

   NcVar *hdr_arr_var = (NcVar *) 0;
   NcVar *hdr_typ_var = (NcVar *) 0;
   NcVar *obs_arr_var = (NcVar *) 0;

   //
   // Open the netCDF point observation file
   //
   cout << "Opening netCDF file: " << nc_file << "\n" << flush;

   f_in = new NcFile(nc_file);

   if(!f_in || !f_in->is_valid()) {
      cerr << "\n\nERROR: main() -> trouble opening netCDF file "
           << nc_file << "\n\n";
      f_in->close();
      delete f_in;
      f_in = (NcFile *) 0;

      exit(1);
   }

   //
   // Retrieve the dimensions and variable from the netCDF file
   //
   hdr_arr_dim = f_in->get_dim("hdr_arr_len");
   obs_arr_dim = f_in->get_dim("obs_arr_len");

   nhdr_dim = f_in->get_dim("nhdr");
   nobs_dim = f_in->get_dim("nobs");

   strl_dim = f_in->get_dim("mxstr");

   hdr_arr_var = f_in->get_var("hdr_arr");
   hdr_typ_var = f_in->get_var("hdr_typ");
   obs_arr_var = f_in->get_var("obs_arr");

   if(!hdr_arr_dim || !hdr_arr_dim->is_valid() ||
      !obs_arr_dim || !obs_arr_dim->is_valid() ||
      !nhdr_dim    || !nhdr_dim->is_valid() ||
      !nobs_dim    || !nobs_dim->is_valid() ||
      !strl_dim    || !strl_dim->is_valid() ||
      !hdr_arr_var || !hdr_arr_var->is_valid() ||
      !hdr_typ_var || !hdr_typ_var->is_valid() ||
      !obs_arr_var || !obs_arr_var->is_valid()) {
      cerr << "\n\nERROR: main() -> trouble reading netCDF file "
           << nc_file << "\n\n";
      exit(1);
   }

   //
   // Allocate space to store the data
   //
   hdr_arr = new float[hdr_arr_dim->size()];
   obs_arr = new float[obs_arr_dim->size()];

   cout << "Processing " << nobs_dim->size() << " observations at "
        << nhdr_dim->size() << " locations.\n" << flush;

   cout << "Observation GRIB codes: " << flush;
   if(ivar.n_elements() == 0) cout << "ALL\n" << flush;
   else {
      for(i=0; i<ivar.n_elements(); i++) cout << ivar[i] << " ";
      cout << "\n" << flush;
   }

   cout << "Observation message types: " << flush;
   if(ityp.n_elements() == 0) cout << "ALL\n" << flush;
   else {
      for(i=0; i<ityp.n_elements(); i++) cout << ityp[i] << " ";
      cout << "\n" << flush;
   }

   //
   // Setup the min/max lat/lon Bounding Box for the grid
   //
   ll_bb.x_ll = -180.0;
   ll_bb.y_ll = -90.0;
   ll_bb.x_ur = 180.0;
   ll_bb.y_ur = 90.0;

   //
   // Create a PostScript file
   //
   cout << "Creating postscript file: " << ps_file << "\n" << flush;
   plot.open(ps_file);
   plot.pagenumber(1);
   plot.setlinewidth(l_width);
   plot.file() << "90 rotate 0 -612 translate";

   //
   // Set plotting dimensions
   //
   dim.x_ll = 20.0;
   dim.x_ur = 772.0;
   dim.y_ll = 20.0;
   dim.y_ur = 552.0;

   plot.choose_font(31, 24.0, met_data_dir);
   plot.write_centered_text(1, 1, 396.0, 572.0, 0.5, 0.5,
                            get_short_name(nc_file));

   //
   // Loop through the observations looking for the correct observation
   // variable type.
   //
   plot.setrgbcolor(1.0, 0.0, 0.0);
   count = 0;
   for(i=0; i<nobs_dim->size(); i++) {

      if(!obs_arr_var->set_cur(i, 0) ||
         !obs_arr_var->get(obs_arr, 1, obs_arr_dim->size())) {
         cerr << "\n\nERROR: main() -> trouble getting obs_arr\n\n";
         exit(1);
      }
      if(obs_arr[0] >= 1.0E10 && obs_arr[1] >= 1.0E10) break;

      //
      // Get the header index and variable type for this observation.
      //
      h = nint(obs_arr[0]);
      v = nint(obs_arr[1]);

      //
      // Check if we want to plot this variable type.
      //
      if(ivar.n_elements() > 0 && !ivar.has(v)) continue;

      //
      // Get the corresponding header message type
      //
      if(!hdr_typ_var->set_cur(h-1, 0) ||
         !hdr_typ_var->get(hdr_typ_str, 1, strl_dim->size())) {
         cerr << "\n\nERROR: main() -> trouble getting hdr_typ\n\n";
         exit(1);
      }

      //
      // Check if we want to plot this message type.
      //
      if(ityp.n_elements() > 0 && !ityp.has(hdr_typ_str)) continue;

      //
      // Only plot a circle if one hasn't been plotted for this
      // location yet.
      //
      if(!ihdr.has(h)) {

         //
         // Get the header for this observation
         //
         hdr_arr_var->set_cur(h-1, 0);
         hdr_arr_var->get(hdr_arr, 1, hdr_arr_dim->size());

         if(hdr_arr[0] >= 1.0E10 && hdr_arr[1] >= 1.0E10) break;

         lat = (double) hdr_arr[0];
         lon = (double) (-1.0*hdr_arr[1]);

         latlon_to_pagexy(grid, xy_bb, lat, lon, page_x, page_y, dim);
         plot.circle(page_x, page_y, 0.5, 0);
         plot.fill();

         ihdr.add(h);
         count++;
      }
   } // end for i
   cout << "Finished plotting " << count << " locations.\n" << flush;

   plot.setrgbcolor(0.0, 0.0, 0.0);
   draw_border(plot, dim);
   draw_map(plot, dim);

   plot.showpage();

   plot.close();

   //
   // Deallocate memory and clean up
   //
   if(f_in)    { f_in->close(); delete f_in; f_in = (NcFile *) 0; }
   if(hdr_arr) { delete hdr_arr; hdr_arr = (float *) 0; }
   if(obs_arr) { delete obs_arr; obs_arr = (float *) 0; }

   return(0);
}

////////////////////////////////////////////////////////////////////////

static void draw_border(PSfile &p, BoundingBox &dim) {

   p.gsave();
   p.setlinewidth(l_width);
   p.newpath();
   p.moveto(dim.x_ll, dim.y_ll);
   p.lineto(dim.x_ur, dim.y_ll);
   p.lineto(dim.x_ur, dim.y_ur);
   p.lineto(dim.x_ll, dim.y_ur);
   p.closepath();
   p.stroke();
   p.grestore();

   return;
}

////////////////////////////////////////////////////////////////////////

static void draw_map(PSfile &p, BoundingBox &dim) {

   p.gsave();
   p.setlinewidth(l_width);
   draw_world(grid, xy_bb, p, ll_bb, dim, c_map, met_data_dir);
   draw_states(grid, xy_bb, p, ll_bb, dim, c_map, met_data_dir);
   p.grestore();

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\nUsage: " << program_name << "\n"
        << "\tnc_file\n"
        << "\tps_file\n"
        << "\t[-gc code]\n"
        << "\t[-msg_typ name]\n"
        << "\t[-data_dir path]\n"
        << "\t[-dim lat_ll lon_ll lat_ur lon_ur]\n\n"

        << "\twhere\t\"nc_file\" is the netCDF point observation file "
        << "to be plotted.\n"
        << "\t\t\"ps_file\" is the name for the output postscript "
        << "file to be generated.\n"
        << "\t\t\"-gc code\" specifies the GRIB code(s) to be plotted "
        << "(optional).\n"
        << "\t\t\"-msg_typ name\" is the message type(s) to be "
        << "plotted (optional).\n"
        << "\t\t\"-data_dir path\" indicates the MET data directory "
        << "to be used (optional).\n"
        << "\t\t\"-dim lat_ll lon_ll lat_ur lon_ur\" defines the "
        << "lat/lon box to be plotted (optional).\n\n"
        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_grib_code(const StringArray & a)
{
   ivar.add(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////

void set_msg_type(const StringArray & a)
{
   ityp.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_data_dir(const StringArray & a)
{
   strcpy(met_data_dir, a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_dim(const StringArray & a)
{
   lat_ll = atof(a[0]);
   lon_ll = atof(a[1])*-1.0;
   lat_ur = atof(a[2]);
   lon_ur = atof(a[3])*-1.0;

   grid.latlon_to_xy(lat_ll, lon_ll, xy_bb.x_ll, xy_bb.y_ll);
   grid.latlon_to_xy(lat_ur, lon_ur, xy_bb.x_ur, xy_bb.y_ur);
   xy_bb.height = xy_bb.y_ur - xy_bb.y_ll;
   xy_bb.width  = xy_bb.x_ur - xy_bb.x_ll;
}

////////////////////////////////////////////////////////////////////////

