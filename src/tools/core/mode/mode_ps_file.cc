// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "mode_ps_file.h"
#include "vx_plot_util.h"

////////////////////////////////////////////////////////////////////////


static const double default_PageWidth  =  8.5*72.0;
static const double default_PageHeight = 11.0*72.0;

static const double default_Hmargin    = 20.0;
static const double default_Vmargin    = 20.0;

static const double default_TextSep    = 15.0;

static const Color default_fcst_fill (150, 150, 150);
static const Color default_obs_fill  (150, 150, 150);

static const Color MapColor      ( 25,  25,  25);
static const Color HullColor     (  0,   0,   0);
static const Color BoundaryColor (  0,   0, 255);

static const Color white(255, 255, 255);

static const int stride = 1;


////////////////////////////////////////////////////////////////////////


static Box valid_xy_bb(const ShapeData * wd_ptr, const Grid & grid);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ModePsFile
   //


////////////////////////////////////////////////////////////////////////


ModePsFile::ModePsFile()

{

mpsf_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ModePsFile::~ModePsFile()

{


}

////////////////////////////////////////////////////////////////////////


ModePsFile::ModePsFile(const ModePsFile &)

{

// mpsf_init_from_scratch();

mlog << Error << "\n\n  ModePsFile::ModePsFile(const ModePsFile &) -> shoule never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


ModePsFile & ModePsFile::operator=(const ModePsFile &)

{

// if ( this == &m )  return ( * this );
//
// assign(m);

mlog << Error << "\n\n  ModePsFile::operator=(const ModePsFile &) -> should never be called!\n\n";

exit ( 1 );

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::mpsf_init_from_scratch()

{

Engine = (ModeFuzzyEngine *) 0;

ConfInfo = (ModeConfInfo *) 0;

grid = (Grid *) 0;

MetDataDir = replace_path("MET_BASE");

PageWidth  = default_PageWidth;
PageHeight = default_PageHeight;

Hmargin = default_Hmargin;
Vmargin = default_Vmargin;

TextSep = default_TextSep;

text_y = 0.0;

FcstFillColor = default_fcst_fill;
ObsFillColor  = default_obs_fill;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::set(ModeFuzzyEngine & e, const Grid & g, double data_min, double data_max)

{

double dmin, dmax;

Engine = &e;

grid = &g;

DataMin = data_min;
DataMax = data_max;

ConfInfo = &(e.conf_info);

SmallPane.set_lrbt(Hmargin, 450.0, 270.0, 720.0);

LargePane.set_lrbt(Hmargin, PageWidth - 3.0*Hmargin, Vmargin, PageHeight - 4.0*Vmargin);

set_xy_box();

set_postscript_dims();

   //
   //  set MetDataDir with the config file value
   //

MetDataDir = ConfInfo->met_data_dir;

   //
   //  load the forecast and obs raw colortables
   //

ConcatString s;

 s = replace_path(ConfInfo->fcst_raw_pi.color_table.c_str());

mlog << Debug(1) << "Loading forecast raw color table: " << s << "\n";

 FcstRawCtable.read(s.c_str());

 s = replace_path(ConfInfo->obs_raw_pi.color_table.c_str());

mlog << Debug(1) << "Loading observation raw color table: " << s << "\n";

 ObsRawCtable.read(s.c_str());

   //
   // If the forecast and observation fields are the same and if the range
   // of both colortables is [0, 1], rescale both colortables to the
   // data_min and data_max values
   //

if ( (ConfInfo->fcst_info->name() == ConfInfo->obs_info->name()) &&
      is_eq( FcstRawCtable.data_min (bad_data_double), 0.0) &&
      is_eq( FcstRawCtable.data_max (bad_data_double), 1.0) &&
      is_eq(  ObsRawCtable.data_min (bad_data_double), 0.0) &&
      is_eq(  ObsRawCtable.data_max (bad_data_double), 1.0) ) {

   FcstRawCtable.rescale (DataMin, DataMax, bad_data_double);
    ObsRawCtable.rescale (DataMin, DataMax, bad_data_double);

} else {

   //
   // Otherwise, if the range of either colortable is [0, 1], rescale
   // the field using the min/max values in the field
   //

   if ( is_eq(FcstRawCtable.data_min(bad_data_double), 0.0) &&
        is_eq(FcstRawCtable.data_max(bad_data_double), 1.0) ) {

      e.fcst_raw->data.data_range(dmin, dmax);

      FcstRawCtable.rescale(dmin, dmax, bad_data_double);

   }

   if ( is_eq(ObsRawCtable.data_min(bad_data_double), 0.0) &&
        is_eq(ObsRawCtable.data_max(bad_data_double), 1.0) ) {

         e.obs_raw->data.data_range(dmin, dmax);

         ObsRawCtable.rescale(dmin, dmax, bad_data_double);

   }

}

   //
   // If the fcst_raw_plot_min or fcst_raw_plot_max value is set in the
   // config file, rescale the forecast colortable to the requested range
   //

if ( !is_eq(ConfInfo->fcst_raw_pi.plot_min, 0.0) ||
     !is_eq(ConfInfo->fcst_raw_pi.plot_max, 0.0) ) {

   FcstRawCtable.rescale(ConfInfo->fcst_raw_pi.plot_min,
                         ConfInfo->fcst_raw_pi.plot_max,
                         bad_data_double);

}

   //
   // If the obs_raw_plot_min or obs_raw_plot_max value is set in the
   // config file, rescale the observation colortable to the requested range
   //

if ( !is_eq(ConfInfo->obs_raw_pi.plot_min, 0.0) ||
     !is_eq(ConfInfo->obs_raw_pi.plot_max, 0.0) ) {

   ObsRawCtable.rescale(ConfInfo->obs_raw_pi.plot_min,
                        ConfInfo->obs_raw_pi.plot_max,
                        bad_data_double);
}

   //
   // Set the fill colors.  If a fill value is not specified in the range
   // of the color table, use the default color.  Otherwise, use the
   // color specified in the color table.
   //

if ( (bad_data_double >= FcstRawCtable.data_min()) &&
     (bad_data_double <= FcstRawCtable.data_max()) ) FcstFillColor = FcstRawCtable.nearest(bad_data_double);

if ( (bad_data_double >= ObsRawCtable.data_min()) &&
     (bad_data_double <= ObsRawCtable.data_max()) )  ObsFillColor = ObsRawCtable.nearest(bad_data_double);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::set_xy_box()

{

   //
   // Check the value of the plot_valid_flag in the config file.
   // If not set, reset the xy_bb to the entire grid.
   //

if(ConfInfo->plot_valid_flag == 0) {

   XY_box.set_llwh(0.0, 0.0, grid->nx(), grid->ny());

   return;

}

   //
   // Compute the x/y bounding box for valid data in each field
   //

double L, R, B, T;
Box fcst_xy_bb, obs_xy_bb;

fcst_xy_bb = valid_xy_bb(Engine->fcst_raw, *grid);
 obs_xy_bb = valid_xy_bb(Engine->obs_raw,  *grid);

   //
   // Compute the x/y bounding box as the union of the
   // fcst and obs x/y bounding boxes
   //

L = min(fcst_xy_bb.left(),   obs_xy_bb.left());
B = min(fcst_xy_bb.bottom(), obs_xy_bb.bottom());
R = max(fcst_xy_bb.right(),  obs_xy_bb.right());
T = max(fcst_xy_bb.top(),    obs_xy_bb.top());

XY_box.set_lrbt(L, R, B, T);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::choose_font(int _font_number, double _font_size)

{

  PSfile::choose_font_with_dir(_font_number, _font_size, MetDataDir.c_str());

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::set_color(const Color & c)

{

double R, G, B;

R = (c.red())/255.0;
G = (c.green())/255.0;
B = (c.blue())/255.0;

setrgbcolor(R, G, B);

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::gridxy_to_pagexy(double x_grid, double y_grid, double & x_page, double & y_page) const

{

   //
   //   This routine depends on the View_box having been already set
   //

::gridxy_to_pagexy(*grid, XY_box, x_grid, y_grid, x_page, y_page, View_box);

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::set_view(double y_ll, double y_ur, double x_cen)

{

double L, R, B, T;
double mag, W;

B = y_ll;
T = y_ur;

mag = (T - B)/(XY_box.height());

W = mag*(XY_box.width());

L = x_cen - 0.5*W;
R = x_cen + 0.5*W;

View_box.set_lrbt(L, R, B, T);


return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::set_postscript_dims()

{

const int nx = nint(XY_box.width());
const int ny = nint(XY_box.height());
double grid_ar, sm_plot_ar, lg_plot_ar;


grid_ar    = ((double) nx)/ny;
sm_plot_ar = (SmallPane.width()/2.0)/(SmallPane.height()/3.0);
lg_plot_ar = (LargePane.width()/1.0)/(LargePane.height()/2.0);

if(grid_ar > sm_plot_ar) SmallPlotHeight = SmallPane.height()/3.0*sm_plot_ar/grid_ar;
else                     SmallPlotHeight = SmallPane.height()/3.0;

if(grid_ar > lg_plot_ar) LargePlotHeight = LargePane.height()/2.0*lg_plot_ar/grid_ar;
else                     LargePlotHeight = LargePane.height()/2.0;

Htab_1 = SmallPane.left()  + SmallPane.width()/4.0;
Htab_2 = SmallPane.left()  + SmallPane.width()/4.0 * 3.0;
Htab_3 = SmallPane.right() + TextSep;

Vtab_1 = SmallPane.top() - SmallPlotHeight;
Vtab_2 = Vtab_1 - SmallPlotHeight;
Vtab_3 = Vtab_2 - SmallPlotHeight;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::outline_box(const Box & b, const double linewidth)

{

gsave();

   setlinewidth(linewidth);

   newpath();

   moveto(b.left(),  b.bottom());
   lineto(b.right(), b.bottom());
   lineto(b.right(), b.top());
   lineto(b.left(),  b.top());

   closepath();
   stroke();

grestore();


return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::outline_view()

{

outline_box(View_box, L_thin);

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::fill_box(const Box & b, const Color & c)

{

gsave();

   set_color(c);

   newpath();

   moveto(b.left(),  b.bottom());
   lineto(b.right(), b.bottom());
   lineto(b.right(), b.top());
   lineto(b.left(),  b.top());

   closepath();
   fill();

grestore();


return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::make_plot()

{

const MergeType fcst_merge_flag = ConfInfo->fcst_merge_flag;
const MergeType  obs_merge_flag = ConfInfo->obs_merge_flag;
ConcatString s;

s << cs_erase
  << "MODE: " << ConfInfo->fcst_info->name() << " at "
  << ConfInfo->fcst_info->level_name() << " vs "
  << ConfInfo->obs_info->name() << " at "
  << ConfInfo->obs_info->level_name();

 plot_engine(*Engine, FOEng, s.c_str());

if ( (fcst_merge_flag == MergeType_Both) || (fcst_merge_flag == MergeType_Thresh) )  {

   plot_threshold_merging(*Engine, "Forecast: Threshold Merging", 1);

}

if ( (fcst_merge_flag == MergeType_Both) || (fcst_merge_flag == MergeType_Engine) )  {

   plot_engine(*(Engine->fcst_engine), FFEng, "Forecast: ModeFuzzyEngine Merging");

}

if ( (obs_merge_flag == MergeType_Both) || (obs_merge_flag == MergeType_Thresh) )  {

   plot_threshold_merging(*Engine, "Observation: Threshold Merging", 0);

}

if ( (obs_merge_flag == MergeType_Both) || (obs_merge_flag == MergeType_Engine) )  {

   plot_engine(*(Engine->obs_engine), OOEng, "Observation: ModeFuzzyEngine Merging");

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::plot_threshold_merging (ModeFuzzyEngine & eng, const char * title , bool fcst)

{

ShapeData merge_mask, merge_split, merge_shape;
Polyline poly;
int n_merge;
double v_tab;
const double h_tab_cen = PageWidth/2.0;

if ( fcst ) {

   merge_mask = *(eng.fcst_conv);
   merge_mask.threshold(eng.conf_info.fcst_merge_thresh);

} else {

   merge_mask = *(eng.obs_conv);
   merge_mask.threshold(eng.conf_info.obs_merge_thresh);

}

merge_split = split(merge_mask, n_merge);

inc_pagenumber();

choose_font(31, 24.0);

write_centered_text(1, 1, h_tab_cen, 752.0, 0.5, 0.5, title);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw raw field
   //
   ////////////////////////////////////////////////////////////////////

v_tab = PageHeight - 3.0*Vmargin;

set_view(v_tab - LargePlotHeight, v_tab, h_tab_cen);

if ( fcst )  {

   comment("threshold merging page: fcst raw");
   render_ppm(eng, FOEng, *(eng.fcst_raw), fcst, 0);

} else {

   comment("threshold merging page: obs raw");
   render_ppm(eng, FOEng, *(eng.obs_raw),  fcst, 0);

}

outline_box(View_box, L_thin);

draw_map( &(eng.conf_info.conf) );

draw_colorbar(fcst);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw split field
   //
   ////////////////////////////////////////////////////////////////////

v_tab -= LargePlotHeight;

set_view(v_tab - LargePlotHeight, v_tab, h_tab_cen);

if ( fcst )  {

   comment("threshold merging page: fcst raw");
   render_ppm(eng, FOEng, *(eng.fcst_split), fcst, 2);

} else {

   comment("threshold merging page: obs split");
   render_ppm(eng, FOEng, *(eng.obs_split),  fcst, 2);

}

outline_box(View_box, L_thin);

draw_map( &(eng.conf_info.conf) );

draw_boundaries(merge_split, n_merge);

   //
   //  done
   //

showpage();

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::draw_boundaries(ModeFuzzyEngine & eng, bool fcst)

{

int i, j;

   //
   // Draw boundaries around each of the objects
   //

if ( fcst ) {

   for (i=0; i<(eng.n_fcst); i++) {

      for (j=0; j<(eng.fcst_single[i].n_bdy); j++) {

         draw_polyline(eng.fcst_single[i].boundary[j], BoundaryColor, false);

      }

   }

   return;
}

   //
   //  obs
   //

for (i=0; i<(eng.n_obs); i++) {

   for (j=0; j<(eng.obs_single[i].n_bdy); j++) {

      draw_polyline(eng.obs_single[i].boundary[j], BoundaryColor, false);

   }

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::draw_boundaries(ShapeData & split_dp, int n_shapes)

{

int i;
Polyline poly;
ShapeData wd_shape;

   //
   // Draw boundary for each shape in the split field
   //

for (i=0; i<n_shapes; i++) {

   wd_shape = select(split_dp, i+1);

   poly = wd_shape.single_boundary();

   draw_polyline(poly, BoundaryColor, false);

}

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::draw_polyline(Polyline & poly, const Color & c, bool latlon)

{

if ( poly.n_points <= 0 ) return;

int i;
double lat, lon;
double grid_x_prev, grid_y_prev, grid_x_cur, grid_y_cur;
double page_x, page_y;


   //
   // Set to specified values
   //

gsave();

setlinewidth(L_thick);
set_color(c);

newpath();

   //
   // Set the starting Grid x,y to the first vertex of
   // the polyline
   //

if ( latlon ) {

   lat = poly.u[0];
   lon = poly.v[0];

   grid->latlon_to_xy(lat, lon, grid_x_prev, grid_y_prev);

} else { // Grid coordinates

   grid_x_prev = poly.u[0];
   grid_y_prev = poly.v[0];

}

   //
   // Convert the starting Grid x/y to page x,y and move to it
   //

gridxy_to_pagexy(grid_x_prev, grid_y_prev, page_x, page_y);

moveto(page_x, page_y);

   //
   // Loop through the vertices and back to the starting vertex
   //

for (i=1; i<poly.n_points; i++) {

      //
      // Draw an arc between the previous Grid x,y point and the
      // current point.
      //

   if ( latlon )  {

      lat = poly.u[i];
      lon = poly.v[i];

      grid->latlon_to_xy(lat, lon, grid_x_cur, grid_y_cur);

   } else { // Grid coordinates

      grid_x_cur = poly.u[i];
      grid_y_cur = poly.v[i];

   }

   gridxy_to_pagexy(grid_x_cur, grid_y_cur, page_x, page_y);

   if ( ConfInfo->plot_gcarc_flag == 0 )  {

      lineto(page_x, page_y);

   } else {

      gc_arcto(*grid, XY_box, *this, grid_x_prev, grid_y_prev, grid_x_cur, grid_y_cur, 10, View_box);

   }

      //
      // Reset the previous Grid x,y to the current Grid x,y
      //

   grid_x_prev = grid_x_cur;
   grid_y_prev = grid_y_cur;

}   //  for i

closepath();
stroke();

grestore();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::draw_map(MetConfig * config)

{

if ( use_zlib )  begin_flate();

gsave();

   setlinewidth(L_thin);

   // ::draw_map(*grid, XY_box, *this, View_box, MapColor, MetDataDir);
   ::draw_map(*grid, XY_box, *this, View_box, config);

grestore();

if ( use_zlib )  end_flate();

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::draw_colorbar(bool fcst)

{

int i, n_colors;
char label[max_str_len];
double bar_width, bar_height, x_ll, y_ll;
ColorTable * ct = (ColorTable *) 0;
Box b;
Color c;

   //
   // Set up the pointer to the appropriate colortable
   //

ct = ( fcst ? (&FcstRawCtable) : (&ObsRawCtable) );

   //
   // Get the number of colortable entries
   //

n_colors = ct->n_entries();

   //
   // Draw colorbar in the bottom right corner of the Bounding Box
   //

gsave();
setlinewidth(L_thin);
choose_font(28, 8.0);

bar_width  = Hmargin/2.0;
bar_height = (View_box.height())/n_colors;

x_ll = View_box.right();
y_ll = View_box.bottom();

ColorTable & ctable = *ct;

for (i=0; i<n_colors; i++) {

   c = ctable[i].color();

   b.set_llwh(x_ll, y_ll, bar_width, bar_height);

     //
     // Color box
     //

   fill_box(b, c);

     //
     // Outline color box
     //

   setgray(0.0);

   outline_box(b, L_thin);

     //
     // Add text
     //

  if ( (i > 0) && (i%stride == 0) )  {

        //
        // Choose the label format based on whether the colortable
        // has been rescaled
        //

     if ( ctable.rescale_flag )   snprintf(label, sizeof(label), "%.2f", ctable[i].value_low());
     else                         snprintf(label, sizeof(label), "%g",   ctable[i].value_low());

     write_centered_text(2, 1,  x_ll + bar_width + 2.0, y_ll, 0.0, 0.5, label);
  }

  y_ll += bar_height;

}   //   for i

grestore();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::render_ppm(ModeFuzzyEngine & eng, EngineType eng_type, const ShapeData &wd, bool fcst, int split)

{

RenderInfo render_info;
Ppm image;
int x, y, v_int;
double mag, v;
Color c;
Color fill_color;
ColorTable *ct = (ColorTable *) 0;
const int L = nint(XY_box.left());
const int B = nint(XY_box.bottom());

   //
   // Set up pointers to the appropriate colortable and fill color values
   //

if ( eng_type == FFEng ) {

   ct         = &FcstRawCtable;
   fill_color =  FcstFillColor;

} else if ( eng_type == OOEng ) {

   ct         = &ObsRawCtable;
   fill_color =  ObsFillColor;

} else { // eng_type == FOEng

   if ( fcst )  {

      ct         = &FcstRawCtable;
      fill_color =  FcstFillColor;

   } else {

      ct         = &ObsRawCtable;
      fill_color =  ObsFillColor;

   }

}

   //
   // Convert the ShapeData object to PPM
   //

image.set_size_xy((int) (XY_box.width()), (int) (XY_box.height()));

for(x=L; x<(XY_box.right()); x++) {

   for(y=B; y<(XY_box.top()); y++) {

      v = wd.data(x, y);

      v_int = nint(v);

      if ( split == 1 )  {  // Single object field

            //
            // Should be no bad data left at this point
            //

              if ( is_bad_data(v) )     c = fill_color;
         else if ( v_int == 0 )         c = white;
         else if ( v_int > 0 &&  fcst ) c = eng.fcst_color[v_int - 1];
         else if ( v_int > 0 && !fcst ) c = eng.obs_color[v_int - 1];

      } else if ( split == 2 )  { // Cluster object field

            //
            // Should be no bad data left at this point
            //

              if ( is_bad_data (v) ) c = fill_color;
         else if ( v_int == 0 )      c = white;
         else                        c = BoundaryColor;

      } else { // Raw data field

         if ( is_bad_data(v) ) c = fill_color;
         else                  c = ct->nearest(v);

      }

      image.putxy(c, x - L, y - B);

   } // end for y

} // end for x

mag = View_box.width()/XY_box.width();

render_info.set_ll(View_box.left(), View_box.bottom());
render_info.set_mag(mag);
render_info.set_color();
// render_info.add_filter(RunLengthEncode);
render_info.add_filter(FlateEncode);
render_info.add_filter(ASCII85Encode);

render(*this, image, render_info);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::draw_convex_hulls(ModeFuzzyEngine & eng, bool fcst, bool id_flag)

{

int i, j;
double grid_x, grid_y, page_x, page_y;
Polyline poly;
Color c;
char label[max_str_len];

gsave();

setlinewidth(0.0);

   //
   // Draw convex hulls around collection of shapes
   //

for (i=0; i<eng.collection.n_sets; i++) {

   if ( fcst ) poly = eng.pair_cluster[i].Fcst[0].convex_hull;
   else        poly = eng.pair_cluster[i].Obs[0].convex_hull;

   // cout << "========= " << (fcst ? "fcst" : "obs") << " " << i << "\n" << flush;
   // poly.dump(cout);

   draw_polyline(poly, HullColor, false);

      //
      // Plot the cluster id
      //

   if(id_flag) {

         //
         // Get the centroid location and color for this set
         //

      if(fcst) {
         grid_x = eng.pair_cluster[i].Fcst[0].centroid_x;
         grid_y = eng.pair_cluster[i].Fcst[0].centroid_y;
         j = eng.collection.set[i].fcst_number[0]-1;
         c = eng.fcst_color[j];
      } else {
         grid_x = eng.pair_cluster[i].Obs[0].centroid_x;
         grid_y = eng.pair_cluster[i].Obs[0].centroid_y;
         j = eng.collection.set[i].obs_number[0]-1;
         c = eng.obs_color[j];
      }

      gridxy_to_pagexy(grid_x, grid_y, page_x, page_y);

      set_color(c);
      choose_font(28, 16.0);
      snprintf(label, sizeof(label), "%i", i+1);
      write_centered_text(2, 1, page_x, page_y, 0.5, 0.5, label);

         //
         // Draw outline in black
         //

      setgray(0.0);

      write_centered_text(2, 0, page_x, page_y, 0.5, 0.5, label);

   }   //  if id_flag

}   //  for i


   //
   //  done
   //

grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::plot_simple_ids(ModeFuzzyEngine & eng, bool fcst)

{

int i, n_shapes;
double grid_x, grid_y, page_x, page_y;
Color c;
char label[max_str_len];

gsave();

if(fcst) n_shapes = eng.n_fcst;
else     n_shapes = eng.n_obs;

setlinewidth(0.0);

for(i=0; i<n_shapes; i++) {

   if ( fcst )  {
      grid_x = eng.fcst_single[i].centroid_x;
      grid_y = eng.fcst_single[i].centroid_y;
   } else {
      grid_x = eng.obs_single[i].centroid_x;
      grid_y = eng.obs_single[i].centroid_y;
   }

   gridxy_to_pagexy(grid_x, grid_y, page_x, page_y);

   if ( fcst )  c = eng.fcst_color[i];
   else         c = eng.obs_color[i];

   set_color(c);
   choose_font(28, 16.0);
   snprintf(label, sizeof(label), "%i", i+1);
   write_centered_text(2, 1, page_x, page_y, 0.5, 0.5, label);

      // Draw outline in black

   setgray(0.0);
   write_centered_text(2, 0, page_x, page_y, 0.5, 0.5, label);

}   //  for i

   //
   //  done
   //

grestore();

return;

}



////////////////////////////////////////////////////////////////////////


void ModePsFile::wct0(double x, double y, const char * text)

{

write_centered_text(1, 1, x, y, 0.0, 0.0, text);

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::wct5(double x, double y, const char * text)

{

write_centered_text(1, 1, x, y, 0.5, 0.0, text);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


Box valid_xy_bb(const ShapeData * wd_ptr, const Grid & grid)

{

int x, y;
int L, R, B, T;
Box bb;

   //
   // Initialize the x/y bounding box
   //

L = grid.nx();
B = grid.ny();
R = 0;
T = 0;

const int data_nx = wd_ptr->data.nx();
const int data_ny = wd_ptr->data.ny();

for(x=0; x<data_nx; x++) {

   for(y=0; y<data_ny; y++) {

      if(wd_ptr->is_valid_xy(x, y)) {

         if(x < L) L = x;
         if(x > R) R = x;

         if(y < B) B = y;
         if(y > T) T = y;

      }

   }

}

bb.set_lrbt(L, R, B, T);

return ( bb );

}


////////////////////////////////////////////////////////////////////////






