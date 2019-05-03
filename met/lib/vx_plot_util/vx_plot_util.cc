// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   vx_plot_util.cc
//
//   Description:
//
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-06-06  Halley Gotway
//
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

#include "vx_plot_util/vx_plot_util.h"

///////////////////////////////////////////////////////////////////////////////

int operator>>(ifstream &in, MapRegion &region)
{
   int j;

   // read in the region number, the number of points for this region number,
   // and the a, b, and c values
   in >> region.number >> region.n_points >> region.a >> region.b >> region.c;

   // check for end of file
   if (!in) return (0);

   // read in the maximum latitude, the minimum latitude, the minimum longitude,
   // and the maximum longitude for this region number
   in >> region.lat_max >> region.lat_min >> region.lon_min >> region.lon_max;

   // check that the number of points to read in is not greater than the size
   // of the arrays to hold the lat/lon point values
   if(region.n_points > max_region_points) {
      cerr << "\n\noperator>>(ifstream &, MapRegion &) ->"
           << "map region has too many points\n\n";
      exit (1);
   }

   // read in the lat/lon point values
   for(j=0; j<region.n_points; j++) {
      in >> region.lat[j] >> region.lon[j];
   }

   return (1);
}

////////////////////////////////////////////////////////////////////////////////

int operator>>(ifstream &in, CountyRegion &region) {
   int j;

   in >> region.a >> region.b >> region.n_points;

   if(!in) return(0);

   in >> region.lat_max >> region.lat_min >> region.lon_min >> region.lon_max;

   if(region.n_points > max_region_points) {
      cerr << "\n\noperator>>(ifstream &, CountyRegion &) -> "
           << "county region has too many points\n\n";
      exit(1);
   }

   for(j=0; j<region.n_points; j++) {
      in >> region.lat[j] >> region.lon[j];
   }

   return(1);
}

////////////////////////////////////////////////////////////////////////////////

void draw_states(const Grid &gr, const BoundingBox &gr_bb, PSfile &p,
                 const BoundingBox &bb, const BoundingBox &dim, Color c) {
   char data_dir[512];

   // By default, set the data_dir to MET_BASE/data
   sprintf(data_dir, "%s/data", MET_BASE);

   draw_states(gr, gr_bb, p, bb, dim, c, data_dir);

   return;
}

////////////////////////////////////////////////////////////////////////////////

void draw_states(const Grid &gr, const BoundingBox &gr_bb, PSfile &p,
                 const BoundingBox &bb, const BoundingBox &dim, Color c,
                 const char *data_dir) {
   ifstream in;
   MapRegion r;
   BoundingBox r_bb;
   bool bb_empty = false;
   char usa_state_file[512];

   sprintf(usa_state_file, "%s/%s", data_dir, usa_state_data);

   if( bb.x_ll < 0.00001 && bb.y_ll < 0.00001 &&
       bb.x_ur < 0.00001 && bb.y_ur < 0.00001 ) bb_empty = true;

   in.open(usa_state_file);

   if(!in) {
      cerr << "\n\ndraw_states() -> unable to open map data file \""
           << usa_state_file << "\"\n\n" << flush;
      exit(1);
   }

   // Setup Clipping path
   p.newpath();
   p.moveto(dim.x_ll, dim.y_ll);
   p.lineto(dim.x_ur, dim.y_ll);
   p.lineto(dim.x_ur, dim.y_ur);
   p.lineto(dim.x_ll, dim.y_ur);
   p.closepath();
   p.gsave();
   p.clip();

   p.file() << "1 setlinejoin\n";
   p.setrgbcolor(c.red()/255.0, c.green()/255.0, c.blue()/255.0);

   while(in >> r) {
      r_bb.x_ll = r.lon_max;
      r_bb.y_ll = r.lat_min;
      r_bb.x_ur = r.lon_min;
      r_bb.y_ur = r.lat_max;

      if(bb_intersect(bb, r_bb) || bb_empty) {
         draw_state_region(gr, gr_bb, p, dim, r);
      }
   }

   p.grestore();

   in.close();

   return;
}

////////////////////////////////////////////////////////////////////////////////

void draw_states(const Grid &gr, PSfile &p,
                 const BoundingBox &bb, const BoundingBox &dim, Color c) {
   BoundingBox gr_bb;

   gr_bb.x_ll = 0;
   gr_bb.y_ll = 0;
   gr_bb.x_ur = gr.nx();
   gr_bb.y_ur = gr.ny();

   gr_bb.width  = gr_bb.x_ur - gr_bb.x_ll;
   gr_bb.height = gr_bb.y_ur - gr_bb.y_ll;

   draw_states(gr, gr_bb, p, bb, dim, c);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_state_region(const Grid &gr, const BoundingBox &gr_bb, PSfile &p,
                       const BoundingBox &dim, const MapRegion &r) {
   int i;
   double px1, py1, px2, py2;

   p.newpath();

   latlon_to_pagexy(gr, gr_bb, r.lat[0], r.lon[0], px1, py1, dim);

   p.moveto(px1, py1);

   for(i=1; i<(r.n_points); i++) {

      latlon_to_pagexy(gr, gr_bb, r.lat[i], r.lon[i], px2, py2, dim);

      //
      // Check for regions which overlap the edge of the grid
      // Finish the previous path and begin a new one
      //
      if(abs(px2 - px1) > 0.90*abs(dim.x_ur - dim.x_ll)) {
         p.stroke();

         p.newpath();

         p.moveto(px2, py2);
      }
      else {
         p.lineto(px2, py2);
      }

      px1 = px2;
      py1 = py2;
   }
   p.stroke();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_state_region(const Grid &gr, PSfile &p,
                       const BoundingBox &dim, const MapRegion &r) {
   BoundingBox gr_bb;

   gr_bb.x_ll = 0;
   gr_bb.y_ll = 0;
   gr_bb.x_ur = gr.nx();
   gr_bb.y_ur = gr.ny();

   gr_bb.width  = gr_bb.x_ur - gr_bb.x_ll;
   gr_bb.height = gr_bb.y_ur - gr_bb.y_ll;

   draw_state_region(gr, gr_bb, p, dim, r);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_counties(const Grid &gr, const BoundingBox &gr_bb, PSfile &p,
                   const BoundingBox &bb, const BoundingBox &dim, Color c) {
   char data_dir[512];

   // By default, set the data_dir to MET_BASE/data
   sprintf(data_dir, "%s/data", MET_BASE);

   draw_counties(gr, gr_bb, p, bb, dim, c, data_dir);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_counties(const Grid &gr, const BoundingBox &gr_bb, PSfile &p,
                   const BoundingBox &bb, const BoundingBox &dim, Color c,
                   const char *data_dir) {
   ifstream in;
   CountyRegion r;
   BoundingBox r_bb;
   char usa_county_file[512];

   sprintf(usa_county_file, "%s/%s", data_dir, usa_county_data);

   in.open(usa_county_file);

   if(!in) {
      cerr << "\n\ndraw_counties() -> unable to open county data file \""
           << usa_county_file << "\"\n\n" << flush;
      exit(1);
   }

   // Setup Clipping path
   p.newpath();
   p.moveto(dim.x_ll, dim.y_ll);
   p.lineto(dim.x_ur, dim.y_ll);
   p.lineto(dim.x_ur, dim.y_ur);
   p.lineto(dim.x_ll, dim.y_ur);
   p.closepath();
   p.gsave();
   p.clip();

   p.file() << "1 setlinejoin\n";
   p.setrgbcolor(c.red()/255.0, c.green()/255.0, c.blue()/255.0);

   while(in >> r) {
      r_bb.x_ll = r.lon_max;
      r_bb.y_ll = r.lat_min;
      r_bb.x_ur = r.lon_min;
      r_bb.y_ur = r.lat_max;

      if(bb_intersect(bb, r_bb)) {
         draw_county_region(gr, gr_bb, p, dim, r);
      }
   }

   p.grestore();

   in.close();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_counties(const Grid &gr, PSfile &p,
                   const BoundingBox &bb, const BoundingBox &dim, Color c) {
   BoundingBox gr_bb;

   gr_bb.x_ll = 0;
   gr_bb.y_ll = 0;
   gr_bb.x_ur = gr.nx();
   gr_bb.y_ur = gr.ny();

   gr_bb.width  = gr_bb.x_ur - gr_bb.x_ll;
   gr_bb.height = gr_bb.y_ur - gr_bb.y_ll;

   draw_counties(gr, gr_bb, p, bb, dim, c);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_county_region(const Grid &gr, const BoundingBox &gr_bb, PSfile &p,
                        const BoundingBox &dim, const CountyRegion &r) {
   int i;
   double px1, py1, px2, py2;

   p.newpath();

   latlon_to_pagexy(gr, gr_bb, r.lat[0], r.lon[0], px1, py1, dim);

   p.moveto(px1, py1);

   for(i=1; i<(r.n_points); i++) {

      latlon_to_pagexy(gr, gr_bb, r.lat[i], r.lon[i], px2, py2, dim);

      //
      // Check for regions which overlap the edge of the grid
      // Finish the previous path and begin a new one
      //
      if(abs(px2 - px1) > 0.90*abs(dim.x_ur - dim.x_ll)) {
         p.stroke();

         p.newpath();

         p.moveto(px2, py2);
      }
      else {
         p.lineto(px2, py2);
      }

      px1 = px2;
      py1 = py2;
   }
   p.stroke();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_county_region(const Grid &gr, PSfile &p,
                        const BoundingBox &dim, const CountyRegion &r) {
   BoundingBox gr_bb;

   gr_bb.x_ll = 0;
   gr_bb.y_ll = 0;
   gr_bb.x_ur = gr.nx();
   gr_bb.y_ur = gr.ny();

   gr_bb.width  = gr_bb.x_ur - gr_bb.x_ll;
   gr_bb.height = gr_bb.y_ur - gr_bb.y_ll;

   draw_county_region(gr, gr_bb, p, dim, r);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_world(const Grid &gr, const BoundingBox &gr_bb, PSfile &p,
                const BoundingBox &bb, const BoundingBox &dim, Color c) {
   char data_dir[512];

   // By default, set the data_dir to MET_BASE/data
   sprintf(data_dir, "%s/data", MET_BASE);

   draw_world(gr, gr_bb, p, bb, dim, c, data_dir);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_world(const Grid &gr, const BoundingBox &gr_bb, PSfile &p,
                const BoundingBox &bb, const BoundingBox &dim, Color c,
                const char *data_dir) {
   ifstream in;
   MapRegion r;
   BoundingBox r_bb;
   bool bb_empty = false;
   char world_outline_file[512];

   sprintf(world_outline_file, "%s/%s", data_dir, world_outline_data);

   if( bb.x_ll < 0.00001 && bb.y_ll < 0.00001 &&
       bb.x_ur < 0.00001 && bb.y_ur < 0.00001 ) bb_empty = true;

   in.open(world_outline_file);

   if(!in) {
      cerr << "\n\ndraw_world() -> unable to open world data file \""
           << world_outline_file << "\"\n\n" << flush;
      exit(1);
   }

   // Setup Clipping path
   p.newpath();
   p.moveto(dim.x_ll, dim.y_ll);
   p.lineto(dim.x_ur, dim.y_ll);
   p.lineto(dim.x_ur, dim.y_ur);
   p.lineto(dim.x_ll, dim.y_ur);
   p.closepath();
   p.gsave();
   p.clip();

   p.file() << "1 setlinejoin\n";
   p.setrgbcolor(c.red()/255.0, c.green()/255.0, c.blue()/255.0);

   while(in >> r) {
      r_bb.x_ll = r.lon_max;
      r_bb.y_ll = r.lat_min;
      r_bb.x_ur = r.lon_min;
      r_bb.y_ur = r.lat_max;

      if(bb_intersect(bb, r_bb) || bb_empty) {
         draw_world_region(gr, gr_bb, p, dim, r);
      }
   }

   p.grestore();

   in.close();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_world(const Grid &gr, PSfile &p,
                const BoundingBox &bb, const BoundingBox &dim, Color c) {
   BoundingBox gr_bb;

   gr_bb.x_ll = 0;
   gr_bb.y_ll = 0;
   gr_bb.x_ur = gr.nx();
   gr_bb.y_ur = gr.ny();

   gr_bb.width  = gr_bb.x_ur - gr_bb.x_ll;
   gr_bb.height = gr_bb.y_ur - gr_bb.y_ll;

   draw_world(gr, gr_bb, p, bb, dim, c);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_world_region(const Grid &gr, const BoundingBox &gr_bb, PSfile &p,
                       const BoundingBox &dim, const MapRegion &r) {
   int i;
   double px1, py1, px2, py2;

   p.newpath();

   latlon_to_pagexy(gr, gr_bb, r.lat[0], r.lon[0], px1, py1, dim);

   p.moveto(px1, py1);

   for(i=1; i<(r.n_points); i++) {

      latlon_to_pagexy(gr, gr_bb, r.lat[i], r.lon[i], px2, py2, dim);

      //
      // Check for regions which overlap the edge of the grid
      // Finish the previous path and begin a new one
      //
      if(abs(px2 - px1) > 0.90*abs(dim.x_ur-dim.x_ll)) {
         p.stroke();

         p.newpath();

         p.moveto(px2, py2);
      }
      else {
         p.lineto(px2, py2);
      }

      px1 = px2;
      py1 = py2;
   }
   p.stroke();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_world_region(const Grid &gr, PSfile &p,
                       const BoundingBox &dim, const MapRegion &r) {
   BoundingBox gr_bb;

   gr_bb.x_ll = 0;
   gr_bb.y_ll = 0;
   gr_bb.x_ur = gr.nx();
   gr_bb.y_ur = gr.ny();

   gr_bb.width  = gr_bb.x_ur - gr_bb.x_ll;
   gr_bb.height = gr_bb.y_ur - gr_bb.y_ll;

   draw_world_region(gr, gr_bb, p, dim, r);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void draw_grid(const Grid &gr, const BoundingBox &gr_bb, int skip, PSfile &p,
               const BoundingBox &dim, Color c) {
   int x, y;
   double page_x, page_y;

   p.gsave();
   p.setrgbcolor(c.red()/255.0, c.green()/255.0, c.blue()/255.0);

   // Draw the vertical grid lines
   for(x=(int) floor(gr_bb.x_ll); x<gr_bb.x_ur; x++) {

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
   for(y=(int) floor(gr_bb.y_ll); y<gr_bb.y_ur; y++) {

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

void draw_grid(const Grid &gr, int skip, PSfile &p,
               const BoundingBox &dim, Color c) {
   BoundingBox gr_bb;

   gr_bb.x_ll = 0;
   gr_bb.y_ll = 0;
   gr_bb.x_ur = gr.nx();
   gr_bb.y_ur = gr.ny();

   gr_bb.width  = gr_bb.x_ur - gr_bb.x_ll;
   gr_bb.height = gr_bb.y_ur - gr_bb.y_ll;

   draw_grid(gr, gr_bb, skip, p, dim, c);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void gc_arcto(const Grid &gr, const BoundingBox &gr_bb, PSfile &p,
              double grid_x_start, double grid_y_start,
              double grid_x_end, double grid_y_end, double dist, const BoundingBox &dim) {
   int i;
   double lat_start, lon_start, lat_end, lon_end, total_dist, num_legs;
   double lat_leg, lon_leg, page_x_leg, page_y_leg;

   gr.xy_to_latlon(grid_x_start, grid_y_start, lat_start, lon_start);
   gr.xy_to_latlon(grid_x_end, grid_y_end, lat_end, lon_end);

   // Calculate the total distance between the points
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

///////////////////////////////////////////////////////////////////////////////

void gc_arcto(const Grid &gr, PSfile &p,
              double grid_x_start, double grid_y_start,
              double grid_x_end, double grid_y_end, double dist, const BoundingBox &dim) {
   BoundingBox gr_bb;

   gr_bb.x_ll = 0;
   gr_bb.y_ll = 0;
   gr_bb.x_ur = gr.nx();
   gr_bb.y_ur = gr.ny();

   gr_bb.width  = gr_bb.x_ur - gr_bb.x_ll;
   gr_bb.height = gr_bb.y_ur - gr_bb.y_ll;

   gc_arcto(gr, gr_bb, p, grid_x_start, grid_y_start, grid_x_end, grid_y_end, dist, dim);

   return;
}

////////////////////////////////////////////////////////////////////////////////

void gridxy_to_pagexy(const Grid &gr, const BoundingBox &gr_bb, double grid_x, double grid_y,
                      double &page_x, double &page_y, const BoundingBox &dim) {

   page_x = dim.x_ll + ((grid_x - gr_bb.x_ll)/gr_bb.width * (dim.x_ur - dim.x_ll));
   page_y = dim.y_ll + ((grid_y - gr_bb.y_ll)/gr_bb.height * (dim.y_ur - dim.y_ll));

   return;
}

////////////////////////////////////////////////////////////////////////////////

void gridxy_to_pagexy(const Grid &gr, double grid_x, double grid_y,
                      double &page_x, double &page_y, const BoundingBox &dim) {
   BoundingBox gr_bb;

   gr_bb.x_ll = 0;
   gr_bb.y_ll = 0;
   gr_bb.x_ur = gr.nx();
   gr_bb.y_ur = gr.ny();

   gr_bb.width  = gr_bb.x_ur - gr_bb.x_ll;
   gr_bb.height = gr_bb.y_ur - gr_bb.y_ll;

   gridxy_to_pagexy(gr, gr_bb, grid_x, grid_y, page_x, page_y, dim);

   return;
}

////////////////////////////////////////////////////////////////////////////////

void latlon_to_pagexy(const Grid &gr, const BoundingBox &gr_bb, double lat, double lon,
                      double &page_x, double &page_y, const BoundingBox &dim) {
   double grid_x, grid_y;

   gr.latlon_to_xy(lat, lon, grid_x, grid_y);

   gridxy_to_pagexy(gr, gr_bb, grid_x, grid_y, page_x, page_y, dim);

   return;
}

////////////////////////////////////////////////////////////////////////////////

void latlon_to_pagexy(const Grid &gr, double lat, double lon,
                      double &page_x, double &page_y, const BoundingBox &dim) {
   BoundingBox gr_bb;

   gr_bb.x_ll = 0;
   gr_bb.y_ll = 0;
   gr_bb.x_ur = gr.nx();
   gr_bb.y_ur = gr.ny();

   gr_bb.width  = gr_bb.x_ur - gr_bb.x_ll;
   gr_bb.height = gr_bb.y_ur - gr_bb.y_ll;

   latlon_to_pagexy(gr, gr_bb, lat, lon, page_x, page_y, dim);

   return;
}

////////////////////////////////////////////////////////////////////////////////
