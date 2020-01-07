// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

using namespace std;

//
// ach_plotting_pkg.cc
//
// Code file for the plotting package classes.
//

#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <limits.h>

#include "vx_log.h"
#include "vx_plot_util.h"

////////////////////////////////////////////////////////////////////////////////
//
// Draw a map in a PostScript file.
//
////////////////////////////////////////////////////////////////////////////////

void draw_map(const Grid &gr, const Box &gr_bb, PSfile &p, const Box &dim,
              MetConfig *conf) {
   int i;
   Dictionary *map_dict = (Dictionary *) 0;
   ConcatString file_name, line_dash;
   NumArray line_color;
   double line_width;

   // Check for a valid dictionary
   if(!conf) {
      mlog << Error << "\ndraw_map() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }
   
   // Lookup the map_data.source dictionary
   map_dict = conf->lookup_array(conf_key_map_data_source);

   // Loop over the map data sources
   for(i=0; i<map_dict->n_entries(); i++) {

      // Get the map data setting for the current entry
      file_name  = replace_path((*map_dict)[i]->dict_value()->lookup_string(conf_key_file_name).c_str());
      line_color = (*map_dict)[i]->dict_value()->lookup_num_array(conf_key_line_color);
      line_width = (*map_dict)[i]->dict_value()->lookup_double(conf_key_line_width);
      line_dash  = (*map_dict)[i]->dict_value()->lookup_string(conf_key_line_dash, false);

      // Check for correctly formatted colors
      if(line_color.n_elements() != 3) {
         mlog << Error << "\ndraw_map() -> "
              << "\"line_color\" must be specified as three RGB values.\n\n";
         exit(1);
      }

      mlog << Debug(4) << "draw_map() -> "
           << "Plotting map data file: " << file_name << "\n";

      // Set the PostScript line options
      p.gsave();
      p.setlinewidth(line_width);
      p.setrgbcolor(line_color[0]/255.0, line_color[1]/255.0, line_color[2]/255.0);
      if(line_dash.length() > 0) p.setdash(line_dash.c_str());
      draw_map_data(gr, gr_bb, p, dim, file_name.c_str());
      p.grestore();
   }
   
   return;
}

////////////////////////////////////////////////////////////////////////////////
//
// Draw map data file.
//   The gr_bb argument specifies the subset of the grid to be plotted.
//   The dim argument specifies the location on the PostScript page.
//
////////////////////////////////////////////////////////////////////////////////

void draw_map_data(const Grid &gr, const Box &gr_bb, PSfile &p,
                   const Box &dim, const char *map_data_file) {
   ifstream in;
   MapRegion r;

  
   in.open(map_data_file);

   if(!in) {
      mlog << Error << "\ndraw_map_data() -> unable to open map data file \""
           << map_data_file << "\"\n\n";
      exit(1);
   }

   // Setup Clipping path
   p.newpath();
   p.moveto(dim.x_ll(), dim.y_ll());
   p.lineto(dim.x_ur(), dim.y_ll());
   p.lineto(dim.x_ur(), dim.y_ur());
   p.lineto(dim.x_ll(), dim.y_ur());
   p.closepath();
   p.gsave();
   p.clip();

   p.file() << "1 setlinejoin\n";

   while(in >> r) draw_region(gr, gr_bb, p, dim, r);

   p.grestore();

   in.close();

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Draw the MapRegion if it overlaps the Grid.
//
///////////////////////////////////////////////////////////////////////////////

void draw_region(const Grid &gr, const Box &gr_bb, PSfile &p,
                 const Box &dim, const MapRegion &r) {
   int i;
   double px1, py1, px2, py2;

   //
   // Return if the current region does not overlap the grid
   //
   if(!region_overlaps_grid(gr, gr_bb, r)) return;

   p.newpath();

   latlon_to_pagexy(gr, gr_bb, r.lat[0], r.lon[0], px1, py1, dim);

   p.moveto(px1, py1);

   for(i=1; i<(r.n_points); i++) {

      latlon_to_pagexy(gr, gr_bb, r.lat[i], r.lon[i], px2, py2, dim);

      //
      // Check for regions which overlap the edge of the grid
      // Finish the previous path and begin a new one
      //
      if(fabs(px2 - px1) > 0.90*fabs(dim.x_ur() - dim.x_ll())) {

         p.stroke();

         p.newpath();

         p.moveto(px2, py2);
      }
      else {
         p.lineto(px2, py2);
      }

      px1 = px2;
      py1 = py2;

   }   //  for i

   p.stroke();

   return;
}

////////////////////////////////////////////////////////////////////////////////

bool region_overlaps_grid(const Grid & grid, const Box & grid_bb,
                          const MapRegion & reg)
{
   int i;
   double x, y;

      //
      // For each point in the region, convert the lat/lon to x/y and
      // check if it is in the grid. If it is, return true, and if not
      // check the next point.
      //
   for (i = 0; i < reg.n_points; i++)
   {
      grid.latlon_to_xy(reg.lat[i], reg.lon[i], x, y);

      if ((x >= grid_bb.x_ll()) && (x < grid_bb.x_ur()) &&
          (y >= grid_bb.y_ll()) && (y < grid_bb.y_ur()))

         return (true);

   }

   return (false);

}

///////////////////////////////////////////////////////////////////////////////

void draw_grid(const Grid &gr, const Box &gr_bb, int skip, PSfile &p,
               const Box &dim, Color c) {
   int x, y;
   double page_x, page_y;

   p.gsave();
   p.setrgbcolor(c.red()/255.0, c.green()/255.0, c.blue()/255.0);

   // Draw the vertical grid lines
   for(x=(int) floor(gr_bb.x_ll()); x<gr_bb.x_ur(); x++) {

      if(x%skip != 0) {
         continue;
      }

      p.newpath();
      gridxy_to_pagexy(gr, gr_bb, x, 0, page_x, page_y, dim);
      p.moveto(page_x, page_y);
      gridxy_to_pagexy(gr, gr_bb, x, gr.ny(), page_x, page_y, dim);
      p.lineto(page_x, page_y);
      p.stroke();
   }

   // Draw the horizontal grid lines
   for(y=(int) floor(gr_bb.y_ll()); y<gr_bb.y_ur(); y++) {

      if(y%skip != 0) {
         continue;
      }

      p.newpath();
      gridxy_to_pagexy(gr, gr_bb, 0, y, page_x, page_y, dim);
      p.moveto(page_x, page_y);
      gridxy_to_pagexy(gr, gr_bb, gr.nx(), y, page_x, page_y, dim);
      p.lineto(page_x, page_y);
      p.stroke();
   }

   p.grestore();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void gc_arcto(const Grid &gr, const Box &gr_bb, PSfile &p,
              double grid_x_start, double grid_y_start,
              double grid_x_end, double grid_y_end, double dist, const Box &dim) {
   int i;
   double lat_start, lon_start, lat_end, lon_end, total_dist, num_legs;
   double lat_leg, lon_leg, page_x_leg, page_y_leg;

   gr.xy_to_latlon(grid_x_start, grid_y_start, lat_start, lon_start);
   gr.xy_to_latlon(grid_x_end, grid_y_end, lat_end, lon_end);

   // Calclate the total distance between the points
   total_dist = gc_dist(lat_start, lon_start, lat_end, lon_end);

   // Calculate the number of legs required
   num_legs = total_dist/dist;

   // Loop through the legs
   for(i=1; i<=num_legs; i++) {

      // Find the lat/lon of the next intermediate point
      gc_point_v1(lat_start, lon_start, lat_end, lon_end, i*dist,
                  lat_leg, lon_leg);

      // Convert the intermediate point to x/y in the map
      latlon_to_pagexy(gr, gr_bb, lat_leg, lon_leg, page_x_leg, page_y_leg, dim);

      // Draw a line to the next intermediate point
      p.lineto(page_x_leg, page_y_leg);
   }

   // Find the end point of the curve and convert to x/y in the map
   gc_point_v1(lat_start, lon_start, lat_end, lon_end, total_dist,
               lat_leg, lon_leg);
   latlon_to_pagexy(gr, gr_bb, lat_leg, lon_leg, page_x_leg, page_y_leg, dim);

   // Draw a line to the last point
   p.lineto(page_x_leg, page_y_leg);

   return;
}

////////////////////////////////////////////////////////////////////////////////
//
// Convert x/y in Grid space to x/y in PostScript page space
//
////////////////////////////////////////////////////////////////////////////////

void gridxy_to_pagexy(const Grid &gr, const Box &gr_bb,
                      double grid_x, double grid_y,
                      double &page_x, double &page_y,
                      const Box &dim) {

   page_x = dim.x_ll() + ((grid_x - gr_bb.x_ll())/gr_bb.width() * (dim.x_ur() - dim.x_ll()));
   page_y = dim.y_ll() + ((grid_y - gr_bb.y_ll())/gr_bb.height() * (dim.y_ur() - dim.y_ll()));

   return;
}

////////////////////////////////////////////////////////////////////////////////
//
// Convert lat/lon to Grid x/y to page x/y
//
////////////////////////////////////////////////////////////////////////////////

void latlon_to_pagexy(const Grid &gr, const Box &gr_bb, double lat, double lon,
                      double &page_x, double &page_y, const Box &dim) {
   double grid_x, grid_y;

   gr.latlon_to_xy(lat, lon, grid_x, grid_y);

   gridxy_to_pagexy(gr, gr_bb, grid_x, grid_y, page_x, page_y, dim);

   return;
}

////////////////////////////////////////////////////////////////////////////////
