// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
//   002    01/03/12  Holmes          Modified to get a grid definition
//                                    from a data file to use for the
//                                    plot.
//   003    01/24/13  Halley Gotway   Add -dotsize command line argument.
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

#include "nc_utils.h"
#include "vx_log.h"
#include "data_plane.h"
#include "write_netcdf.h"
#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "vx_data2d_grib.h"
#include "vx_data2d_nc_met.h"
#include "vx_data2d_nc_pinterp.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_grid.h"
#include "vx_math.h"
#include "vx_color.h"
#include "vx_ps.h"
#include "vx_pxm.h"
#include "vx_render.h"
#include "vx_plot_util.h"
#include "nc_obs_util.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char  *program_name = "plot_point_obs";
static const Color  c_map(25, 25, 25);
static const double l_width = 0.5;
static const double default_dotsize = 1.0;

static const double margin_size = 36.0;

static const bool use_flate = true;


static int   obs_hid_block[DEF_NC_BUFFER_SIZE];
static int   obs_vid_block[DEF_NC_BUFFER_SIZE];
static float obs_lvl_block[DEF_NC_BUFFER_SIZE];
static float obs_hgt_block[DEF_NC_BUFFER_SIZE];
static float obs_val_block[DEF_NC_BUFFER_SIZE];
static float obs_arr_block[DEF_NC_BUFFER_SIZE][OBS_ARRAY_LEN];

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static Grid         grid("G004");
static Box          grid_bb;
static StringArray  ityp;
static IntArray     ivar;
static StringArray  svar;
static StringArray  var_list;
static ConcatString data_plane_filename;
static double       dotsize = default_dotsize;

///////////////////////////////////////////////////////////////////////////////

static void draw_border(PSfile &, Box &);
static void usage();
static void set_grib_code(const StringArray &);
static void set_obs_var(const StringArray &);
static void set_msg_type(const StringArray &);
static void set_data_filename(const StringArray &);
static void set_dotsize(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   CommandLine cline;
   ConcatString nc_file, ps_file;
   ConcatString met_data_dir;
   int i, h, v, plot_count, skip_count;
   PSfile plot;
   Box page, view, map_box;
   double lat, lon, grid_x, grid_y, page_x, page_y, mag;
   IntArray ihdr;

   //
   // check for zero arguments
   //
   if(argc == 1) usage();

   //
   // Initialize the grid bounding box to the whole grid
   //
   grid_bb.set_llwh( 0.0, 0.0, grid.nx(), grid.ny() );

   //
   // Initialize the data directory using MET_BASE
   //
   met_data_dir = replace_path(default_met_data_dir);

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
   cline.add(set_obs_var, "-obs_var", 1);
   cline.add(set_msg_type, "-msg_typ", 1);
   cline.add(set_data_filename, "-data_file", 1);
   cline.add(set_dotsize, "-dotsize", 1);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);

   //
   // parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be two arguments left; the
   // netCDF point observation filename and the output PostScript
   // filename.
   //
   if(cline.n() != 2) usage();

   //
   // Store the input arguments
   //
   nc_file = cline[0];
   ps_file = cline[1];

   //
   // check if the user entered a data_plane_filename to use a specific
   // grid definition
   //
   if (data_plane_filename.length() > 0)
   {

      mlog << Debug(2) << "Retrieving grid from file: "
           << data_plane_filename << "\n";

      //
      // instantiate a Met2dDataFile object using the data_2d_factory
      // and get the grid info from the file.
      //
      Met2dDataFile * met_ptr = (Met2dDataFile *) 0;
      Met2dDataFileFactory m_factory;

      met_ptr = m_factory.new_met_2d_data_file(data_plane_filename.c_str());

      if (!met_ptr)
      {
         mlog << Error << "\n" << program_name << " -> file \""
              << data_plane_filename << "\" not a valid data file\n\n";
         exit (1);
      }

      grid = met_ptr->grid();

      if ( met_ptr )  { delete met_ptr;  met_ptr = 0; }

   } else {
      mlog << Debug(2) << "Using default global grid.\n";
   }

   //
   // Declare NetCDF file, dimensions, and variables
   //
   NcFile *f_in       = (NcFile *) 0;

   //
   // Open the netCDF point observation file
   //
   mlog << Debug(1) << "Opening netCDF file: " << nc_file << "\n";

   f_in = open_ncfile(nc_file.c_str());

   if(!f_in || IS_INVALID_NC_P(f_in)) {
      mlog << Error << "\nmain() -> trouble opening netCDF file "
           << nc_file << "\n\n";
      delete f_in;
      f_in = (NcFile *) 0;

      exit(1);
   }

   // Read the dimensions and variables
   NetcdfObsVars obsVars;
   read_nc_dims_vars(obsVars, f_in);
   bool use_var_id = obsVars.use_var_id;

   if(IS_INVALID_NC(obsVars.hdr_dim)    ||
      IS_INVALID_NC(obsVars.obs_dim)    ||
      IS_INVALID_NC(obsVars.strl_dim)    ||
      IS_INVALID_NC(obsVars.hdr_typ_var) ||
      IS_INVALID_NC(obsVars.hdr_sid_var) ||
      IS_INVALID_NC(obsVars.hdr_vld_var)) {
      mlog << Error << "\nmain() -> "
           << "trouble reading netCDF file " << nc_file << "\n\n";
      exit(1);
   }

   if (is_version_less_than_1_02(f_in)) {
      if(IS_INVALID_NC(obsVars.hdr_arr_dim) ||
         IS_INVALID_NC(obsVars.obs_arr_dim) ||
         IS_INVALID_NC(obsVars.hdr_arr_var) ||
         IS_INVALID_NC(obsVars.obs_arr_var)) {
         mlog << Error << "\nmain() -> "
              << "trouble reading netCDF file " << nc_file << "(2D variables)\n\n";
         exit(1);
      }
   }
   else {
      if(IS_INVALID_NC(obsVars.hdr_lat_var) ||
         IS_INVALID_NC(obsVars.hdr_typ_tbl_var) ||
         IS_INVALID_NC(obsVars.obs_qty_tbl_var) ||
         IS_INVALID_NC(obsVars.obs_val_var)) {
         mlog << Error << "\nmain() -> "
              << "trouble reading netCDF file " << nc_file << "(header or obs)\n\n";
         exit(1);
      }
   }

   long nhdr_count  = get_dim_size(&obsVars.hdr_dim);
   long nobs_count  = get_dim_size(&obsVars.obs_dim);
   mlog << Debug(2) << "Processing " << (nobs_count) << " observations at "
        << nhdr_count << " locations.\n";

   // Get the corresponding header (message type, staton_id, valid_time, and lat/lon/elv)
   NcHeaderData header_data = get_nc_hdr_data(obsVars);
   int  strl_len = header_data.strl_len;
   int  typ_len  = header_data.typ_len;
   int  sid_len  = header_data.sid_len;
   int  vld_len  = header_data.vld_len;

   if(use_var_id) {
      if(ivar.n_elements() != 0) {
         mlog << Warning << "\n-gc option is ignored!\n\n";
      }
      mlog << Debug(2) << "Observation var names: ";
      if (svar.n_elements() == 0) mlog << "ALL";
      else {
         for(i=0; i<svar.n_elements(); i++) mlog << svar[i] << " ";
      }
   }
   else {
      if(svar.n_elements() != 0) {
         mlog << Warning << "\n-obs_var option is ignored!\n\n";
      }
      mlog << Debug(2) << "Observation GRIB codes: ";
      if(ivar.n_elements() == 0) mlog << "ALL";
      else {
         for(i=0; i<ivar.n_elements(); i++) mlog << ivar[i] << " ";
      }
   }
   mlog << "\n";

   mlog << Debug(2) << "Observation message types: ";
   if(ityp.n_elements() == 0) mlog << "ALL\n";
   else {
      for(i=0; i<ityp.n_elements(); i++) mlog << ityp[i] << " ";
      mlog << "\n";
   }

   //
   // Setup the min/max lat/lon Bounding Box for the grid
   //
   grid_bb.set_llwh(0, 0, grid.nx(), grid.ny());

   //
   // Create a PostScript file
   //
   mlog << Debug(1) << "Creating postscript file: " << ps_file << "\n";
   plot.open(ps_file.c_str(), OrientationLandscape);
   plot.pagenumber(1);
   plot.setlinewidth(l_width);

   //
   // Set plotting dimensions
   //
   page.set_llwh(0.0, 0.0, plot.page_width(), plot.page_height());

   view.set_llwh(margin_size, margin_size, page.width() - 2.0 * margin_size, page.height() - 3.0 * margin_size);

   //
   // calculate how much to magnify the map to get it to fill the view box
   // without distorting the map. e.g. it will either bump the top and bottom
   // of the view box or bump the left and right sides of the view box or both.
   //

   mag = calc_mag(grid_bb, view);

   map_box.set_llwh(view.left() + 0.5 * view.width() - 0.5 * mag * grid_bb.width(),
                    view.bottom() + 0.5 * view.height() - 0.5 * mag * grid_bb.height(),
                    mag * grid_bb.width(), mag * grid_bb.height());

   //
   // annotate the plot with the filename
   //
   plot.choose_font(31, 24.0);
   plot.write_centered_text(1, 1, 0.5 * page.width(), map_box.top() + margin_size, 0.5, 0.5,
                            get_short_name(nc_file.c_str()));

   //
   // read config file defaults
   //
   MetConfig config;
   config.read(replace_path(config_const_filename).c_str());
   config.read(replace_path(config_map_data_filename).c_str());

   //
   // draw the map first and then put a border around it
   //
   plot.setrgbcolor(0.0, 0.0, 0.0);

   if ( use_flate )  plot.begin_flate();
   draw_map(grid, grid_bb, plot, map_box, &config);

   draw_border(plot, map_box);

   //
   // set up clipping path so that the circles will not be made
   // outside the map box
   //
   plot.gsave();
   plot.newpath();
   plot.moveto(map_box.x_ll(), map_box.y_ll());
   plot.lineto(map_box.x_ur(), map_box.y_ll());
   plot.lineto(map_box.x_ur(), map_box.y_ur());
   plot.lineto(map_box.x_ll(), map_box.y_ur());
   plot.clip();

   bool use_hdr_arr = !IS_INVALID_NC(obsVars.hdr_arr_var);
   bool use_obs_arr = !IS_INVALID_NC(obsVars.obs_arr_var);
   int hdr_arr_len = use_hdr_arr ? get_dim_size(&obsVars.hdr_arr_dim) : HDR_ARRAY_LEN;
   int obs_arr_len = use_obs_arr ? get_dim_size(&obsVars.obs_arr_dim) : OBS_ARRAY_LEN;

   int buf_size = ((nobs_count > DEF_NC_BUFFER_SIZE) ? DEF_NC_BUFFER_SIZE : (nobs_count));

   //
   // Allocate space to store the data
   //
   char hdr_typ_str[typ_len];
   char hdr_sid_str[sid_len];
   char hdr_vld_str[vld_len];

   float hdr_arr[hdr_arr_len];
   float obs_arr[obs_arr_len];

   //
   // Loop through the observations looking for the correct observation
   // variable type.
   //
   plot.setrgbcolor(1.0, 0.0, 0.0);
   plot_count = skip_count = 0;

   long offsets[2] = { 0, 0 };
   long lengths[2] = { 1, 1 };
   long offsets_1D[1] = { 0 };
   long lengths_1D[1] = { 1 };

   if (use_var_id) {
      NcDim bufr_var_dim = get_nc_dim(f_in, nc_dim_nvar);
      long var_count = get_dim_size(&bufr_var_dim);
      char obs_var_str[var_count][strl_len];
      NcVar obs_var_var = get_nc_var(f_in, nc_var_obs_var);

      lengths[0] = var_count;
      lengths[1] = strl_len;
      if(!get_nc_data(&obs_var_var, (char *)&obs_var_str[0], lengths, offsets)) {
         mlog << Error << "\nmain() -> "
              << "trouble getting " << nc_var_obs_var << "\n\n";
         exit(1);
      }
      for (int index = 0; index<var_count; index++) {
         var_list.add(obs_var_str[index]);
      }
   }

   for(int i_start=0; i_start<nobs_count; i_start+=buf_size) {
      buf_size = ((nobs_count-i_start) > DEF_NC_BUFFER_SIZE) ? DEF_NC_BUFFER_SIZE : (nobs_count-i_start);

      offsets[0] = i_start;
      lengths[0] = buf_size;
      offsets_1D[0] = i_start;
      lengths_1D[0] = buf_size;
      if (use_obs_arr) {
         lengths[1] = obs_arr_len;

         // Read the current observation message
         if(!get_nc_data(&obsVars.obs_arr_var, (float *)&obs_arr_block[0], lengths, offsets)) {
            mlog << Error << "\nmain() -> trouble getting obs_arr\n\n";
            exit(1);
         }
      }
      else {
         // Read the current observation message
         if(!get_nc_data(&obsVars.obs_hid_var, (int *)&obs_hid_block[0], lengths_1D, offsets_1D)) {
            mlog << Error << "\nmain() -> trouble getting obs_hid\n\n";
            exit(1);
         }
         if(!get_nc_data((IS_INVALID_NC(obsVars.obs_vid_var)
                  ? &obsVars.obs_gc_var :&obsVars.obs_vid_var),
               (int *)&obs_vid_block[0], lengths_1D, offsets)) {
            mlog << Error << "\nmain() -> trouble getting obs_hid\n\n";
            exit(1);
         }
         if(!get_nc_data(&obsVars.obs_lvl_var, (float *)&obs_lvl_block[0], lengths_1D, offsets_1D)) {
            mlog << Error << "\nmain() -> trouble getting obs_lvl\n\n";
            exit(1);
         }
         if(!get_nc_data(&obsVars.obs_hgt_var, (float *)&obs_hgt_block[0], lengths_1D, offsets_1D)) {
            mlog << Error << "\nmain() -> trouble getting obs_hgt\n\n";
            exit(1);
         }
         if(!get_nc_data(&obsVars.obs_val_var, (float *)&obs_val_block[0], lengths_1D, offsets_1D)) {
            mlog << Error << "\nmain() -> trouble getting obs_val\n\n";
            exit(1);
         }
      }

      int hdr_idx;
      for(int i_offset=0; i_offset<buf_size; i_offset++) {
         int str_length;
         i = i_start + i_offset;

         if (use_obs_arr) {
            for (int j=0; j < obs_arr_len; j++)
               obs_arr[j] = obs_arr_block[i_offset][j];
         }
         else {
            obs_arr[0] = (float)obs_hid_block[i_offset];
            obs_arr[1] = (float)obs_vid_block[i_offset];
            obs_arr[2] = obs_lvl_block[i_offset];
            obs_arr[3] = obs_hgt_block[i_offset];
            obs_arr[4] = obs_val_block[i_offset];
         }

         if(obs_arr[0] >= 1.0E10 && obs_arr[1] >= 1.0E10) break;

         //
         // Get the header index and variable type for this observation.
         //
         h = nint(obs_arr[0]);
         v = nint(obs_arr[1]);

         hdr_arr[0] = header_data.lat_array[h];
         hdr_arr[1] = header_data.lon_array[h];
         hdr_arr[2] = header_data.elv_array[h];

         hdr_idx = use_obs_arr ? h : header_data.typ_idx_array[h];
         str_length = header_data.typ_array[hdr_idx].length();
         if (str_length >= typ_len) str_length = typ_len - 1;
         strncpy(hdr_typ_str, header_data.typ_array[hdr_idx].c_str(), str_length);
         hdr_typ_str[str_length] = bad_data_char;

         hdr_idx = use_obs_arr ? h : header_data.sid_idx_array[h];
         str_length = header_data.sid_array[hdr_idx].length();
         if (str_length >= sid_len) str_length = sid_len - 1;
         strncpy(hdr_sid_str, header_data.sid_array[hdr_idx].c_str(), str_length);
         hdr_sid_str[str_length] = bad_data_char;

         hdr_idx = use_obs_arr ? h : header_data.vld_idx_array[h];
         str_length = header_data.vld_array[hdr_idx].length();
         if (str_length >= vld_len) str_length = vld_len - 1;
         strncpy(hdr_vld_str, header_data.vld_array[hdr_idx].c_str(), str_length);
         hdr_vld_str[str_length] = bad_data_char;

         //
         // Check if we want to plot this variable type.
         //
         if (use_var_id) {
            if(svar.n_elements() > 0 && !svar.has(var_list[v])) continue;
         }
         else {
            if(ivar.n_elements() > 0 && !ivar.has(v)) continue;
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
            if(hdr_arr[0] >= 1.0E10 && hdr_arr[1] >= 1.0E10) break;

            lat = (double) hdr_arr[0];
            lon = (double) (-1.0*hdr_arr[1]);

            //
            // Convert lat/lon to grid x/y
            //
            grid.latlon_to_xy(lat, lon, grid_x, grid_y);

            //
            // If the current point is off the grid, increment the skip count
            //
            if(grid_x < 0 || grid_x >= grid.nx() ||
               grid_y < 0 || grid_y >= grid.ny()) {
               skip_count++;
               ihdr.add(h);
               continue;
            }

            //
            // Convert grid x/y to page x/y
            //
            gridxy_to_pagexy(grid, grid_bb, grid_x, grid_y, page_x, page_y, map_box);

            //
            // Draw a circle at this point and increment the plot count
            //
            plot.circle(page_x, page_y, dotsize, 0);
            plot.fill();

            //
            // Dump out the location being plotted
            //
            mlog << Debug(3)
                 << "[" << plot_count + 1
                 << "] Plotting location [ type, sid, valid, lat, lon, elevation ] = [ "
                 << hdr_typ_str << ", " << hdr_sid_str << ", " << hdr_vld_str << ", "
                 << hdr_arr[0] << ", " << hdr_arr[1] << ", " << hdr_arr[2] << " ]\n";

            ihdr.add(h);
            plot_count++;
         }
      }

   } // end for i
   plot.grestore();

   if ( use_flate )  plot.end_flate();

   mlog << Debug(2)
        << "Finished plotting " << plot_count << " locations.\n"
        << "Skipped " << skip_count << " locations off the grid.\n";

   plot.showpage();

   plot.close();

   //
   // Deallocate memory and clean up
   //
   if(f_in)    {
      delete f_in; f_in = (NcFile *) 0;
   }
   header_data.typ_idx_array.clear();
   header_data.sid_idx_array.clear();
   header_data.vld_idx_array.clear();
   header_data.typ_array.clear();
   header_data.sid_array.clear();
   header_data.vld_array.clear();
   header_data.lat_array.clear();
   header_data.lon_array.clear();
   header_data.elv_array.clear();

   return(0);
}

////////////////////////////////////////////////////////////////////////

static void draw_border(PSfile &p, Box &dim) {

   p.gsave();
   p.setlinewidth(l_width);
   p.newpath();
   p.moveto(dim.x_ll(), dim.y_ll());
   p.lineto(dim.x_ur(), dim.y_ll());
   p.lineto(dim.x_ur(), dim.y_ur());
   p.lineto(dim.x_ll(), dim.y_ur());
   p.closepath();
   p.stroke();
   p.grestore();

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\nUsage: " << program_name << "\n"
        << "\tnc_file\n"
        << "\tps_file\n"
        << "\t[-gc code] or [-obs_var variable name]\n"
        << "\t[-msg_typ name]\n"
        << "\t[-data_file name]\n"
        << "\t[-dotsize val]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"nc_file\" is the input NetCDF point observation "
        << "file to be plotted.\n"
        << "\t\t\"ps_file\" is the output PostScript image file to be"
        << "generated.\n"
        << "\t\t\"-gc code\" is the GRIB code(s) to be plotted "
        << "(optional).\n"
        << "\t\t\"-obs_var variable name\" is the variable name(s) to be plotted "
        << "(optional).\n"
        << "\t\t\"-msg_typ name\" is the message type(s) to be "
        << "plotted (optional).\n"
        << "\t\t\"-data_file name\" is a data file whose grid should be "
        << "used for the plot (optional).\n"
        << "\t\t\"-dotsize val\" overrides the default dotsize value ("
        << default_dotsize << ") (optional).\n"
        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"
        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"
        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_grib_code(const StringArray & a)
{
   ivar.add(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_obs_var(const StringArray & a)
{
   svar.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_msg_type(const StringArray & a)
{
   ityp.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_data_filename(const StringArray & a)
{
   data_plane_filename = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_dotsize(const StringArray & a)
{
   dotsize = atof(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

