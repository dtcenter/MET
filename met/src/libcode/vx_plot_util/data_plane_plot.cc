// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////


static const int num_cbar_vals = 300;

static const double one_inch = 72.0;

static const int num_ticks = 9;

static const bool use_flate = true;


///////////////////////////////////////////////////////////////////////////////

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   data_plane_plot.cc
//
//   Description:
//
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    12-28-11  Holmes
//
///////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <limits.h>

#include "vx_log.h"
#include "data_plane_plot.h"

////////////////////////////////////////////////////////////////////////////////


static void create_image(Ppm &, const Grid &, const DataPlane &, const ColorTable &);

static void draw_border(PSfile &, const Box &, double linewidth);

static void fill_colorbar_image(Ppm &, const ColorTable &);


////////////////////////////////////////////////////////////////////////////////

void data_plane_plot(const ConcatString & inname, const ConcatString & outname,
                     const Grid & grid, const ConcatString & title,
                     const ColorTable & colortable, MetConfig *conf,
                     const DataPlane & data_plane)
{
   PSfile plot;
   Ppm image, cbar_image;
   RenderInfo render_info, cbar_render_info;
   Box grid_bb, image_bb, cbar_bb;
   Box page, view, map_box;
   ConcatString short_filename;
   ConcatString junk;
   int i;
   double mag;
   double tick_m, val_m, tick_val, x1, x2, y1;

   short_filename = get_short_name(inname.c_str());

      //
      // open the PostScript file using the default media
      // and landscape mode
      //
   plot.open(outname.c_str(), OrientationLandscape);

      //
      // get plot started by setting page number
      //
   plot.pagenumber(1);

      //
      // set the bounding boxes for boxes used
      // Note: the page width and height are adjusted for
      // orientation in the PSfile class.
      // page is the size of the paper to use in PostScript points.
      // view is the viewport which is a centered box on the page with a
      // one inch margin around it.
      // image_bb is the box for the image which is left justified in the
      // view box leaving a one inch space on the right for the color bar.
      // grid_bb is the size of the data grid.
      //
      // There are two more boxes set below:
      // max_box is the box in which the image and map will be drawn.
      // cbar_bb is the box for the colorbar which is placed one-eighth
      // of an inch to the right of the map box and is one half inch wide.
      //

   page.set_llwh(0.0, 0.0, plot.page_width(), plot.page_height());

   view.set_llwh(one_inch, one_inch, page.width() - 2.0 * one_inch, page.height() - 2.0 * one_inch);

   image_bb.set_llwh(view.left(), view.bottom(), view.width() - one_inch, view.height());

   grid_bb.set_llwh(0.0, 0.0, grid.nx(), grid.ny());

      //
      // calculate how much to magnify the image to get it to fill the image box
      // without distorting the image. e.g. it will either bump the top and bottom
      // of the image box or bump the left and right sides of the image or both.
      //

   mag = calc_mag(grid_bb, image_bb);

   map_box.set_llwh(image_bb.left() + 0.5 * image_bb.width() - 0.5 * mag * grid_bb.width(),
                    image_bb.bottom() + 0.5 * image_bb.height() - 0.5 * mag * grid_bb.height(),
                    mag * grid_bb.width(), mag * grid_bb.height());

   cbar_bb.set_llwh(map_box.right() + 0.125 * one_inch, map_box.bottom(), 0.5 * one_inch, map_box.height());

      //
      // create the data image
      //
   image.set_size_xy(grid.nx(), grid.ny());
   create_image(image, grid, data_plane, colortable);

      //
      // set the render information
      //

   render_info.set_mag(mag);

   render_info.set_ll(map_box.left(), map_box.bottom());

   if ( use_flate )  render_info.add_filter(FlateEncode);
   render_info.add_filter(ASCII85Encode);

      //
      // render the data image
      //

   plot.comment("drawing data image");

   render(plot, image, render_info);

      //
      // draw the map on top of the image and put a border around it
      //

   plot.comment("start drawing map");

   if ( use_flate )  plot.begin_flate();
   draw_map(grid, grid_bb, plot, map_box, conf);
   if ( use_flate )  plot.end_flate();

   plot.comment("end drawing map");

   draw_border(plot, map_box, 2.0);

      //
      // annotate the plot with the filename and the title if set
      //

      //
      // choose the font
      //
   plot.choose_font(11, 12.0);  // 11 = Helvetica and size 12.0

      //
      // center the title, if it exists, 1/2 inch above the top of the image
      //
      // 1 means to center horizontally, 1 means to fill the text,
      // 0.5 * page_width() means to center the text on the page,
      // map_box.top() + 0.5 * one_inch means to place the text .5 inch above the top
      // of the image, and the delta_x at 0.5 means to center the text at the
      // (x,y) point, and the delta_y at 1.0 means to place the top of the text
      // at the (x,y) point.
      //
   if (title.length() > 0)
      plot.write_centered_text(1, 1, 0.5 * page.width(), map_box.top() + 0.5 * one_inch, 0.5, 1.0, title.c_str());

      //
      // put the filename 1/2 inch below the bottom of the image and beginning at
      // the left edge
      //
      // 1 means to center horizontally, 1 means to fill the text,
      // map_box.left() means to begin the text at the left corner of the image,
      // map_box.bottom() - 0.5 * one_inch means to place the text .5 inch below
      // the bottom of the image, and the delta_x at 0.0 means to place the
      // text at the (x,y) point, and the delta_y at 0.0 means to place the
      // bottom of the text at the (x,y) point.
      //
   plot.write_centered_text(1, 1, map_box.left(), map_box.bottom() - 0.5 * one_inch, 0.0, 0.0, short_filename.c_str());

      //
      // now fill in the colorbar and draw a border around it
      //

   plot.comment("drawing colorbar image");

   cbar_image.set_size_xy(1, num_cbar_vals);
   fill_colorbar_image(cbar_image, colortable);
   draw_border(plot, cbar_bb, 1.5);

      //
      // set the render information for the colorbar
      //
   cbar_render_info.set_mag(cbar_bb.width(), map_box.height() / num_cbar_vals);

   cbar_render_info.set_ll(cbar_bb.left(), cbar_bb.bottom());

   cbar_render_info.add_filter(ASCII85Encode);

      //
      // render the data image
      //
   render(plot, cbar_image, cbar_render_info);

      //
      // annotate the colorbar
      //

      //
      // choose the font
      //
   plot.choose_font(11, 8.0);  // 11 = Helvetica and size 8.0

   tick_m = (cbar_bb.top() - cbar_bb.bottom()) / (num_ticks - 1);

   val_m = (colortable.data_max(bad_data_double) - colortable.data_min(bad_data_double)) / (num_ticks - 1);

   for (i = 0; i < num_ticks; i++)
   {
         //
         // first add some tick marks
         //
      x1 = cbar_bb.right();
      y1 = tick_m * i + cbar_bb.bottom();
      x2 = x1 + 5;

      plot.line(x1, y1, x2, y1, true);

         //
         // now put the value centered at this tick mark
         //
      tick_val = val_m * i + colortable.data_min(bad_data_double);

      junk.format("%g", tick_val);

         //
         // 2 means to center text both horizontally and vertically,
         // 1 means to fill the text,
         // x2 + 2.5 means to begin the text 2.5 pixels to the right of the x2 point,
         // y1 means to place the text at the y1 tick mark, and the delta_x at 0.0 means to place the
         // text at the (x,y) point, and the delta_y at 0.5 means to place the
         // center of the text at the (x,y) point.
         //
      plot.write_centered_text(2, 1, x2 + 2.5, y1, 0.0, 0.5, junk.c_str());

   }

      //
      // done, clean up, close file and return
      //
   plot.showpage();
   plot.close();

}

////////////////////////////////////////////////////////////////////////////////

void create_image(Ppm & image, const Grid & grid, const DataPlane & plane,
                  const ColorTable & ctable)
{
   int x, y;
   double value;
   Color color;

      //
      // run thru all the data points and put them into the ppm
      // image
      //
   for (y = 0; y < grid.ny(); y++)
   {
      for (x = 0; x < grid.nx(); x++)
      {
            //
            // get data value from grib record for this point
            //
         value = plane.get(x, y);

         if(is_bad_data(value)) value = bad_data_double;

            //
            // set the color for this value
            //
         color = ctable.interp(value);

            //
            // put this color into the ppm image at this point
            //
         image.putxy(color, x, y);
      }
   }

}

////////////////////////////////////////////////////////////////////////////////

void draw_border(PSfile & plot, const Box & dim, double linewidth)
{

   plot.gsave();
   plot.setlinewidth(linewidth);
   plot.newpath();
   plot.moveto(dim.x_ll(), dim.y_ll());
   plot.lineto(dim.x_ur(), dim.y_ll());
   plot.lineto(dim.x_ur(), dim.y_ur());
   plot.lineto(dim.x_ll(), dim.y_ur());
   plot.closepath();
   plot.stroke();
   plot.grestore();

}

////////////////////////////////////////////////////////////////////////////////

void fill_colorbar_image(Ppm & image, const ColorTable & ctable)
{
   Color color;
   int i;
   double value;
   double dmin, dmax;
   double m;

      //
      // get the min and max values for the colortable
      //
   dmin = ctable.data_min(bad_data_double);
   dmax = ctable.data_max(bad_data_double);

   m = (dmax - dmin)/(num_cbar_vals - 1);

   for (i = 0; i < num_cbar_vals; i++)
   {
         //
         // get data value
         //
      value = m*i + dmin;

         //
         // set the color for this value
         //
      color = ctable.interp(value);

         //
         // put this color into the ppm image at this point
         //
      image.putxy(color, 0, i);

   }


}

////////////////////////////////////////////////////////////////////////////////
