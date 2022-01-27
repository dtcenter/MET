// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
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
//   000    06/10/10  Halley Gotway   New
//   001    10/19/11  Holmes          Added use of command line class to
//                                    parse the command line arguments.
//   002    01/03/12  Holmes          Modified to get a grid definition
//                                    from a data file to use for the
//                                    plot.
//   003    01/24/13  Halley Gotway   Add -dotsize.
//   004    11/10/20  Halley Gotway   Add -config and -plot_grid.
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

#include "plot_point_obs.h"

#include "data_plane_plot.h"
#include "vx_ps.h"
#include "vx_pxm.h"
#include "vx_render.h"
#include "vx_plot_util.h"
#include "nc_point_obs_in.h"

////////////////////////////////////////////////////////////////////////

static void usage();
static void process_point_obs(const char *);
static void create_plot();
static void add_colorbar(PSfile &, const Box &, const ColorTable &);
static void set_config(const StringArray &);
static void set_point_obs(const StringArray &);
static void set_plot_grid(const StringArray &);
static void set_title(const StringArray &);
static void set_grib_code(const StringArray &);
static void set_obs_var(const StringArray &);
static void set_msg_type(const StringArray &);
static void set_dotsize(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   CommandLine cline;

   // Check for zero arguments
   if(argc == 1) usage();

   // Parse the command line into tokens
   cline.set(argc, argv);

   // Set the usage function
   cline.set_usage(usage);

   // Add the options function calls
   cline.add(set_config,    "-config",    1);
   cline.add(set_point_obs, "-point_obs", 1);
   cline.add(set_plot_grid, "-plot_grid", 1);
   cline.add(set_title,     "-title",     1);
   cline.add(set_grib_code, "-gc",        1);
   cline.add(set_obs_var,   "-obs_var",   1);
   cline.add(set_msg_type,  "-msg_typ",   1);
   cline.add(set_dotsize,   "-dotsize",   1);

   // Quietly support old -data_file option
   cline.add(set_plot_grid, "-data_file", 1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be two arguments left:
   // Input point observation file and output plot filename
   if(cline.n() != 2) usage();

   // Store the input arguments
   nc_file.insert(0, cline[0].c_str());
   ps_file = cline[1];

   // Read and process the config file
   conf_info.read_config(config_filename.c_str());
   conf_info.process_config(plot_grid_string.c_str());

   mlog << Debug(3) << "Plotting grid definition: "
        << conf_info.grid.serialize() << "\n";

   // Overwrite configuration using command line options:
   //    -msg_typ, -gc, -obs_var, -dotsize
   if(ityp.n() > 0)          conf_info.set_msg_typ(ityp);
   if(ivar.n() > 0)          conf_info.set_obs_gc (ivar);
   if(svar.n() > 0)          conf_info.set_obs_var(svar);
   if(!is_bad_data(dotsize)) conf_info.set_dotsize(dotsize);

   // Loop over and process each point observation file
   for(int i=0; i<nc_file.n(); i++) {
      process_point_obs(nc_file[i].c_str());
   }

   // Plot the result
   create_plot();

   return(0);
}

////////////////////////////////////////////////////////////////////////

void process_point_obs(const char *point_obs_filename) {
   int h, v, i_obs;
   const char *method_name = "process_point_obs() -> ";
   const char *method_name_s = "process_point_obs() ";

   // Open the netCDF point observation file
   mlog << Debug(1) << "Reading point observation file: "
        << point_obs_filename << "\n";

   bool status;
   bool use_var_id = true;
   bool use_obs_arr = false;

   bool use_python = false;
   MetNcPointObsIn nc_point_obs;
   MetPointData *met_point_obs = 0;
#ifdef WITH_PYTHON
   MetPythonPointDataFile met_point_file;
   string python_command = point_obs_filename;
   bool use_xarray = (0 == python_command.find(conf_val_python_xarray));
   use_python = use_xarray || (0 == python_command.find(conf_val_python_numpy));
   if (use_python) {
      int offset = python_command.find("=");
      if (offset == std::string::npos) {
         mlog << Error << "\n" << method_name
              << "trouble parsing the python command " << python_command << ".\n\n";
         exit(1);
      }

      if(!met_point_file.open(python_command.substr(offset+1).c_str(), use_xarray)) {
         met_point_file.close();
         mlog << Error << "\n" << method_name
              << "trouble getting point observation file from python command "
              << python_command << ".\n\n";
         exit(1);
      }

      met_point_obs = met_point_file.get_met_point_data();
   }
   else
#endif
   {
      if(!nc_point_obs.open(point_obs_filename)) {
         nc_point_obs.close();

         mlog << Error << "\n" << method_name
              << "trouble opening point observation file "
              << point_obs_filename << ".\n\n";

         exit(1);
      }

      // Read the dimensions and variables
      nc_point_obs.read_dim_headers();
      nc_point_obs.check_nc(point_obs_filename, method_name_s);   // exit if missing dims/vars
      nc_point_obs.read_obs_data();

      use_obs_arr = nc_point_obs.is_using_obs_arr();

      // Print warning about ineffective command line arguments
      if(use_var_id && ivar.n() != 0) {
         mlog << Warning << "\n" << method_name << "-gc option is ignored!\n\n";
      }
      if(!use_var_id && svar.n() != 0) {
         mlog << Warning << "\n" << method_name << "-obs_var option is ignored!\n\n";
      }
      met_point_obs = (MetPointData *)&nc_point_obs;
   }

   long nhdr_count = met_point_obs->get_hdr_cnt();
   long nobs_count = met_point_obs->get_obs_cnt();

   mlog << Debug(2) << "Processing " << nobs_count
        << " observations at " << nhdr_count << " locations.\n";

   // Get the corresponding header:
   //   message type, staton_id, valid_time, and lat/lon/elv

   int buf_size = (nobs_count > DEF_NC_BUFFER_SIZE) ? DEF_NC_BUFFER_SIZE : nobs_count;

   // Allocate space to store the data
   float hdr_arr[HDR_ARRAY_LEN];
   float obs_arr[OBS_ARRAY_LEN];
   int obs_qty_block[buf_size];
   float obs_arr_block[buf_size][OBS_ARRAY_LEN];

   use_var_id = met_point_obs->is_using_var_id();
   if(use_var_id) var_list = met_point_obs->get_var_names();
   qty_list = met_point_obs->get_qty_data();

   if (0 == qty_list.n()) {
      mlog << Error << "\n" << method_name << "Missing quality marks\n\n";
      exit (1);
   }
   if (use_var_id && (0 == var_list.n())) {
      mlog << Debug(1) << method_name << "Missing variable names\ns\n";
      exit (1);
   }

   ConcatString hdr_typ_str;
   ConcatString hdr_sid_str;
   ConcatString hdr_vld_str;
   ConcatString obs_qty_str;

   for(int i_start=0; i_start<nobs_count; i_start+=buf_size) {
      int buf_size2 = ((nobs_count-i_start) > DEF_NC_BUFFER_SIZE) ?
                        DEF_NC_BUFFER_SIZE : (nobs_count-i_start);

#ifdef WITH_PYTHON
      if (use_python)
         status = met_point_obs->get_point_obs_data()->fill_obs_buf(
                             buf_size2, i_start, (float *)obs_arr_block, obs_qty_block);
      else
#endif
      status = nc_point_obs.read_obs_data(buf_size2, i_start, (float *)obs_arr_block,
                                          obs_qty_block, (char *)0);
      if (!status) exit(1);

      int typ_idx, sid_idx, vld_idx;
      for(int i_offset=0; i_offset<buf_size2; i_offset++) {

         i_obs = i_start + i_offset;
         for (int j=0; j<OBS_ARRAY_LEN; j++) {
            obs_arr[j] = obs_arr_block[i_offset][j];
         }

         if(obs_arr[0] >= 1.0E10 && obs_arr[1] >= 1.0E10) break;

         int qty_offset = use_obs_arr ? i_obs : obs_qty_block[i_offset];
         obs_qty_str = qty_list[qty_offset];

         // Get the header index and variable type for this observation.
         h = nint(obs_arr[0]);
         v = nint(obs_arr[1]);

         // Read the corresponding header array for this observation
         // - the corresponding header type, header Station ID, and valid time
#ifdef WITH_PYTHON
         if (use_python)
            met_point_obs->get_header(h, hdr_arr, hdr_typ_str, hdr_sid_str, hdr_vld_str);
         else
#endif
            nc_point_obs.get_header(h, hdr_arr, hdr_typ_str, hdr_sid_str, hdr_vld_str);

         // Store data in an observation object
         Observation cur_obs(
            hdr_typ_str,          // message type
            hdr_sid_str,          // station id
            timestring_to_time_t(                    // valid time
               hdr_vld_str.c_str()),
            hdr_arr[0],                // latitude
            hdr_arr[1],                // longitude
            hdr_arr[2],                // elevation
            obs_qty_str.c_str(),                     // quality string
            (use_var_id ? bad_data_int : v),         // grib code
            (double) obs_arr[2],                     // pressure
            (double) obs_arr[3],                     // height
            (double) obs_arr[4],                     // value
            (use_var_id ? var_list[v] : na_string)); // variable name

         // Add this observation to this list
         conf_info.add(cur_obs);

      } // end for i_offset
   } // end for i_start

   // Clean up
#ifdef WITH_PYTHON
   if (use_python) met_point_file.close();
   else
#endif
   nc_point_obs.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void create_plot() {
   int i, plot_count, skip_count;
   PSfile plot;
   Box grid_bb, page, view, map_box, cbar_box;
   double lat, lon, grid_x, grid_y, page_x, page_y, mag, size;
   ColorTable ct;
   vector<PlotPointObsOpt>::iterator it_ppo;
   vector<LocationInfo>::iterator it_loc;

   // Setup the min/max lat/lon Bounding Box for the grid
   grid_bb.set_llwh(0, 0, conf_info.grid.nx(), conf_info.grid.ny());

   // Create a PostScript file
   mlog << Debug(1) << "Creating postscript file: " << ps_file << "\n";
   plot.open(ps_file.c_str(), OrientationLandscape);
   plot.pagenumber(1);
   plot.setlinewidth(line_width);

   // Set plotting dimensions
   page.set_llwh(0.0, 0.0, plot.page_width(), plot.page_height());

   // Need more room for a colorbar
   if(conf_info.do_colorbar) {
      view.set_llwh(1.0 * one_inch, 0.5 * one_inch,
                    page.width()  - 2.5 * one_inch,
                    page.height() - 1.5 * one_inch);
   }
   else {
      view.set_llwh(0.5 * one_inch, 0.5 * one_inch,
                    page.width()  - 1.0 * one_inch,
                    page.height() - 1.5 * one_inch);
   }

   // Calculate how much to magnify the map to get it to fill the view
   // box without distorting the map. e.g. it will either bump the top
   // and bottome of the view box or bump the left and right sides of
   // the view box or both.
   mag = calc_mag(grid_bb, view);

   map_box.set_llwh(
      view.left()   + 0.5*view.width()  - 0.5*mag*grid_bb.width(),
      view.bottom() + 0.5*view.height() - 0.5*mag*grid_bb.height(),
      mag*grid_bb.width(), mag*grid_bb.height());

   cbar_box.set_llwh(map_box.right() + 0.125 * one_inch,
                     map_box.bottom(), 0.5   * one_inch,
                     map_box.height());

   // Add a title string
   ConcatString cs;
   if(title_string.nonempty()) {
      cs = title_string;
   }
   else {
      cs = get_short_name(nc_file[0].c_str());
      for(i=1; i<nc_file.n(); i++) {
         cs << ", " << get_short_name(nc_file[i].c_str());
      }
   }
   plot.choose_font(31, 24.0);
   plot.write_centered_text(1, 1, 0.5 * page.width(),
                            map_box.top() + 0.5 * one_inch, 0.5, 0.5,
                            cs.c_str());
   
   // Plot gridded data, if provided
   if(!conf_info.grid_data.is_empty()) {

      // Read the color table
      ct.read(replace_path(
                 conf_info.grid_plot_info.color_table).c_str());

      // Rescale to the user-specified range
      if(!is_eq(conf_info.grid_plot_info.plot_min, 0.0) ||
         !is_eq(conf_info.grid_plot_info.plot_max, 0.0)) {
         ct.rescale(conf_info.grid_plot_info.plot_min,
                    conf_info.grid_plot_info.plot_max,
                    bad_data_double);
      }
      // Rescale to the actual range of the data
      else if(is_eq(ct.data_min(bad_data_double), 0.0) &&
              is_eq(ct.data_max(bad_data_double), 1.0)) {

         double data_min, data_max;
         conf_info.grid_data.data_range(data_min, data_max);
         ct.rescale(data_min, data_max, bad_data_double);
      }

      // Create the image
      Ppm image;
      image.set_size_xy(conf_info.grid.nx(), conf_info.grid.ny());
      create_image(image, conf_info.grid, conf_info.grid_data, ct);
       
      // Set the render info
      RenderInfo render_info;
      render_info.set_mag(mag);
      render_info.set_ll(map_box.left(), map_box.bottom());
      if(use_flate) render_info.add_filter(FlateEncode);
      render_info.add_filter(ASCII85Encode);

      // Render the data image
      plot.comment("drawing data image");
      render(plot, image, render_info);

      // Draw a colorbar, if specified
      if(conf_info.grid_plot_info.colorbar_flag) {
         add_colorbar(plot, cbar_box, ct);
      }
   }

   // Loop through the options and add a colorbar
   for(it_ppo = conf_info.point_opts.begin();
       it_ppo != conf_info.point_opts.end(); it_ppo++) {

      // Draw a colorbar, if specified
      if(it_ppo->fill_plot_info.flag &&
         it_ppo->fill_plot_info.colorbar_flag) {
         add_colorbar(plot, cbar_box, it_ppo->fill_ctable);
      }
   }
    
   // Draw the map first and then put a border around it
   plot.setrgbcolor(0.0, 0.0, 0.0);

   if(use_flate) plot.begin_flate();

   draw_map(conf_info.grid, grid_bb, plot, map_box, &conf_info.conf);

   draw_border(plot, map_box, line_width);

   // Set up clipping path so that the circles will not be made
   // outside the map box
   plot.gsave();
   plot.newpath();
   plot.moveto(map_box.x_ll(), map_box.y_ll());
   plot.lineto(map_box.x_ur(), map_box.y_ll());
   plot.lineto(map_box.x_ur(), map_box.y_ur());
   plot.lineto(map_box.x_ll(), map_box.y_ur());
   plot.clip();

   // Plot the locations for each set of plotting options
   plot_count = skip_count = 0;
   for(i = 0, it_ppo = conf_info.point_opts.begin();
       it_ppo != conf_info.point_opts.end(); it_ppo++) {

      mlog << Debug(3) << "For point data group " << ++i
           << ", plotting " << it_ppo->locations.size()
           << " locations for " << it_ppo->n_obs << " observations.\n";

      // Loop over the locations
      for(it_loc = it_ppo->locations.begin();
          it_loc != it_ppo->locations.end(); it_loc++) {

         // Convert lat/lon to grid x/y
         lat = (double) it_loc->lat;
         lon = (double) (-1.0*it_loc->lon);
         conf_info.grid.latlon_to_xy(lat, lon, grid_x, grid_y);

         // Track the number of points off the grid
         if(grid_x < 0 || grid_x >= conf_info.grid.nx() ||
            grid_y < 0 || grid_y >= conf_info.grid.ny()) {
            skip_count++;
            continue;
         }

         // Convert grid x/y to page x/y
         gridxy_to_pagexy(conf_info.grid, grid_bb, grid_x, grid_y,
                          page_x, page_y, map_box);

         // Get the size of the circle
         size = it_ppo->dotsize_fx(it_loc->val);

         // Draw a circle and fill it
         if(it_ppo->fill_point) {
            plot.set_color(it_ppo->fill_plot_info.flag ?
                           it_ppo->fill_ctable.nearest(it_loc->val) :
                           it_ppo->fill_color);
            plot.circle(page_x, page_y, size, false);
            plot.fill();
         }
        
         // Outline the circle
         if(it_ppo->outline_point) {
            plot.setlinewidth(it_ppo->line_width);
            plot.set_color(it_ppo->line_color);
            plot.circle(page_x, page_y, size, true);
         }

         // Dump out the location being plotted
         mlog << Debug(4) << "[" << ++plot_count
              << "] Plotting location [ lat, lon, val ] = [ "
              << it_loc->lat << ", " << it_loc->lon << ", "
              << it_loc->val << " ]\n";

      } // end locations
   } // end point_opts

   if(use_flate) plot.end_flate();

   mlog << Debug(2)
        << "Finished plotting " << plot_count << " locations for " << ps_file << ".\n"
        << "Skipped " << skip_count << " locations off the grid.\n";

   plot.showpage();

   plot.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void add_colorbar(PSfile &plot, const Box &box, const ColorTable &ct) {

   // Only plot one colorbar
   if(added_colorbar) {
      mlog << Warning << "\nadd_colorbar()-> "
           << "only one colorbar can be plotted.\n\n";
      return;
   }

   added_colorbar = true;

   double tick_m, val_m, x1, x2, y1;
   ConcatString cs;

   plot.comment("drawing colorbar image");

   Ppm image;
   image.set_size_xy(1, num_colorbar_vals);
   fill_colorbar_image(image, ct);
   draw_border(plot, box, 1.5);

   RenderInfo render_info;
   render_info.set_mag(box.width(), box.height() / num_colorbar_vals);
   render_info.set_ll(box.left(), box.bottom());
   render_info.add_filter(ASCII85Encode);
   render(plot, image, render_info);

   // Annotate the colorbar
   plot.choose_font(11, 8.0);

   tick_m = (box.top() - box.bottom()) / (num_ticks - 1);
   val_m  = (ct.data_max(bad_data_double) -
             ct.data_min(bad_data_double)) /
            (num_ticks - 1);

   for(int i=0; i<num_ticks; i++) {

      // First add some tick marks
      x1 = box.right();
      y1 = tick_m * i + box.bottom();
      x2 = x1 + 5;

      plot.line(x1, y1, x2, y1, true);

      // Put the value centered at this tick mark
      cs.format("%g", val_m * i + ct.data_min(bad_data_double));

      plot.write_centered_text(2, 1, x2 + 2.5, y1, 0.0, 0.5,
                               cs.c_str());
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\nUsage: " << program_name << "\n"
        << "\tnc_file\n"
        << "\tps_file\n"
        << "\t[-config config_file]\n"
        << "\t[-point_obs file]\n"
        << "\t[-plot_grid name]\n"
        << "\t[-title string]\n"
        << "\t[-gc code] or [-obs_var name]\n"
        << "\t[-msg_typ name]\n"
        << "\t[-dotsize val]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"nc_file\" is the input NetCDF point observation "
        << "file to be plotted.\n"
        << "\t\t\"ps_file\" is the output PostScript image file to be"
        << "generated.\n"
        << "\t\t\"-config config_file\" specifies a PlotPointObs "
        << "config file defining plotting options (optional).\n"
        << "\t\t\"-point_obs file\" specifies additional point "
        << "observation files to be plotted (optional).\n"
        << "\t\t\tNote that command line settings override config file "
        << "options.\n"
        << "\t\t\"-plot_grid string\" is a named grid, gridded data "
        << "file, or grid specification string (optional).\n"
        << "\t\t\"-gc code\" is the GRIB code(s) to be plotted "
        << "(optional).\n"
        << "\t\t\"-title string\" specifies the plot title string "
        << "(optional).\n"
        << "\t\t\"-obs_var name\" is the variable name to be plotted "
        << "(optional).\n"
        << "\t\t\"-msg_typ name\" is the message type to be plotted "
        << "(optional).\n"
        << "\t\t\"-dotsize val\" sets a constant dotsize value "
        << "(optional).\n"
        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"
        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"
        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray & a) {
    config_filename = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_point_obs(const StringArray & a) {
   nc_file.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_plot_grid(const StringArray & a) {
   plot_grid_string = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_title(const StringArray & a) {
   title_string = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_grib_code(const StringArray & a) {
   ivar.add(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_obs_var(const StringArray & a) {
   svar.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_msg_type(const StringArray & a) {
   ityp.add(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_dotsize(const StringArray & a) {
   dotsize = atof(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////
