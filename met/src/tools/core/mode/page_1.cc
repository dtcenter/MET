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
#include "mode_ps_table_defs.h"
#include "table_helper.h"


////////////////////////////////////////////////////////////////////////


static double Htab_a, Htab_b, Htab_c;

static const double dx = 2.0;
static const double dy = 2.0;

static double table_bottom = 10.0;

static const int max_interest_rows = 29;


////////////////////////////////////////////////////////////////////////


void ModePsFile::do_page_1(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

int j;
char junk[1024];
ConcatString label, thresh_str;
ConcatString tmp1_str, tmp2_str, tmp3_str;
int i, mon, day, yr, hr, minute, sec;
unixtime t;


   ////////////////////////////////////////////////////////////////////
   //
   // First Page: create a 6 plot
   //
   ////////////////////////////////////////////////////////////////////

inc_pagenumber();

set_family(ff_Palatino);

setlinecap(1);
setlinejoin(1);

choose_font(31, 24.0);

write_centered_text(1, 1, 306.0, 752.0, 0.5, 0.5, title);

choose_font(31, 18.0);

write_centered_text(1, 1, Htab_1, 727.0, 0.5, 0.5, FcstString.c_str());
write_centered_text(1, 1, Htab_2, 727.0, 0.5, 0.5, ObsString.c_str());

   ////////////////////////////////////////////////////////////////////
   //
   // Draw raw forecast field
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_1, Vtab_1 + SmallPlotHeight, Htab_1);
comment("fcst raw");
render_ppm( eng, eng_type, *(eng.fcst_raw), 1, 0);
outline_view();
draw_map( &(eng.conf_info.conf) );

   ////////////////////////////////////////////////////////////////////
   //
   // Draw raw observation field
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_1, Vtab_1 + SmallPlotHeight, Htab_2);
comment("obs raw");
render_ppm(eng, eng_type, *(eng.obs_raw), 0, 0);
outline_view();
draw_map( &(eng.conf_info.conf) );

   ////////////////////////////////////////////////////////////////////
   //
   // Draw fcst split field
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_2, Vtab_2 + SmallPlotHeight, Htab_1);
comment("fcst split");
render_ppm(eng, eng_type, *(eng.fcst_split), 1, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 1, 0);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw obs split field
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_2, Vtab_2 + SmallPlotHeight, Htab_2);
comment("obs split");
render_ppm(eng, eng_type, *eng.obs_split, 0, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 0, 0);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw fcst simple ids
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_3, Vtab_3 + SmallPlotHeight, Htab_1);
outline_view();
draw_map( &(eng.conf_info.conf) );
plot_simple_ids(eng, 1);

   ////////////////////////////////////////////////////////////////////
   //
   // Draw obs simple ids
   //
   ////////////////////////////////////////////////////////////////////

set_view(Vtab_3, Vtab_3 + SmallPlotHeight, Htab_2);
outline_view();
draw_map( &(eng.conf_info.conf) );
plot_simple_ids(eng, 0);

   ////////////////////////////////////////////////////////////////////
   //
   // Plot interest values
   //
   ////////////////////////////////////////////////////////////////////

choose_font(31, 11.0);

setlinewidth(0.2);

text_y = 727.0 - 1.5*TextSep;

TableHelper t0;
double x;
int r, c, n;
int bad_count;
double table_width = 130.0;

n = (eng.n_fcst)*(eng.n_obs);

if ( n > max_interest_rows )  n = max_interest_rows;

t0.set(*this, n + 1, 3);

for (j=0; j<(t0.nrows()); ++j)  t0.set_row_height(j, 15.0);

t0.set_col_width(2, 50.0);
t0.set_col_width(0, 0.5*(table_width - t0.col_width(2)));
t0.set_col_width(1, t0.col_width(0));

t0.set_pin(Htab_3, Vtab_1 + SmallPlotHeight + TextSep, 0.0, 1.0);

t0.fill_row(0, light_green);


bad_count = 0;

for(i=0; i<n; i++) {

   if ( eng.info_singles[i].interest_value < eng.conf_info.total_interest_thresh )  ++bad_count;

}

for (j=0; j<bad_count; ++j)  {

   t0.fill_row(t0.nrows() - 1 - j, light_gray);

}

t0.draw_skeleton(0.2);
t0.outline_table(0.2, black);

bold();

t0.write_xy1_to_cell(0, 0, dx, dy, 0.0, 0.0, FcstShortString.c_str());
t0.write_xy1_to_cell(0, 1, dx, dy, 0.0, 0.0,  ObsShortString.c_str());
t0.write_xy1_to_cell(0, 2, dx, dy, 0.0, 0.0,  "Interest");

roman();

x = 0.5*(t0.col_width(0));

for(i=0; i<n; i++) {

   snprintf(junk, sizeof(junk), "%d", eng.info_singles[i].fcst_number);

   t0.write_xy1_to_cell(i + 1, 0, x, dy, 0.5, 0.0, junk);

   snprintf(junk, sizeof(junk), "%d", eng.info_singles[i].obs_number);

   t0.write_xy1_to_cell(i + 1, 1, x, dy, 0.5, 0.0, junk);

   if ( eng.info_singles[i].interest_value < 0 ) label = na_str;
   else {
      label.set_precision(4);
      label << cs_erase << eng.info_singles[i].interest_value;
   }

   t0.write_xy1_to_cell(i + 1, 2, dx, dy, 0.0, 0.0, label.c_str());

}

   ////////////////////////////////////////////////////////////////////
   //
   // Model Name, Initialization, Valid, Lead, and Accumulation Times
   //
   ////////////////////////////////////////////////////////////////////

set_font_size(11.0);

text_y   = Vtab_3 - 1.0*TextSep;
Htab_a = Htab_1 - 0.5*View_box.width();
Htab_b = Htab_a + 4.0*TextSep;
Htab_c = Htab_a + 9.0*TextSep;

TableHelper t1;


t1.set(*this, 10, 3);

for (j=0; j<(t1.nrows()); ++j)  t1.set_row_height(j, 15.0);

t1.set_col_width(0, Htab_b - Htab_a);
t1.set_col_width(1, Htab_c - Htab_b);
t1.set_col_width(2, Htab_c - Htab_b);

t1.set_pin(Htab_a, Vtab_3 - 2.0, 0.0, 1.0);

table_bottom = t1.bottom();

t1.fill_cell(0, 1, light_green);
t1.fill_cell(0, 2, light_green);

t1.fill_cell(1, 0, blue1);
t1.fill_cell(3, 0, blue1);
t1.fill_cell(5, 0, blue1);
t1.fill_cell(6, 0, blue1);
t1.fill_cell(9, 0, blue1);

t1.fill_cell(2, 0, blue2);
t1.fill_cell(4, 0, blue2);
t1.fill_cell(7, 0, blue2);
t1.fill_cell(8, 0, blue2);

t1.fill_cell(0, 0, dark_gray);

r = 2; t1.fill_cell(r, 1, light_gray); t1.fill_cell(r, 2, light_gray);
r = 4; t1.fill_cell(r, 1, light_gray); t1.fill_cell(r, 2, light_gray);
r = 7; t1.fill_cell(r, 1, light_gray); t1.fill_cell(r, 2, light_gray);
r = 8; t1.fill_cell(r, 1, light_gray); t1.fill_cell(r, 2, light_gray);

r = 0; line(t1.left(), t1.row_bottom(r), t1.right(), t1.row_bottom(r));
r = 1; line(t1.left(), t1.row_bottom(r), t1.right(), t1.row_bottom(r));
r = 2; line(t1.left(), t1.row_bottom(r), t1.right(), t1.row_bottom(r));
r = 3; line(t1.left(), t1.row_bottom(r), t1.right(), t1.row_bottom(r));
r = 4; line(t1.left(), t1.row_bottom(r), t1.right(), t1.row_bottom(r));
r = 6; line(t1.left(), t1.row_bottom(r), t1.right(), t1.row_bottom(r));
r = 8; line(t1.left(), t1.row_bottom(r), t1.right(), t1.row_bottom(r));

c = 0; line(t1.col_right(c), t1.top(), t1.col_right(c), t1.bottom());
c = 1; line(t1.col_right(c), t1.top(), t1.col_right(c), t1.row_bottom(0));
       line(t1.col_right(c), t1.row_bottom(1), t1.col_right(c), t1.bottom());

t1.outline_table(0.2, black);

   //
   // Field name
   //

nextline();

bold();

t1.write_xy1_to_cell(0, 1, dx, dy, 0.0, 0.0, FcstString.c_str());
t1.write_xy1_to_cell(0, 2, dx, dy, 0.0, 0.0,  ObsString.c_str());

r = 1;

c = 0;

t1.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0,  "Model");
t1.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0,  "Field");
t1.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0,  "Level");
t1.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0,  "Units");
t1.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0,  "Initial");
++r;
t1.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0,  "Valid");
++r;
t1.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0,  "Accum");

roman();

   //
   // Model Name
   //

nextline();

// t1.write_xy1_to_cell(1, 1, dx, dy, 0.0, 0.0, eng.conf_info.model);
t1.write_xy1_to_cell(1, 2, 0.0, dy, 0.5, 0.0, eng.conf_info.model.c_str());

   //
   // Variable Name
   //

nextline();

t1.write_xy1_to_cell(2, 1, dx, dy, 0.0, 0.0, eng.conf_info.fcst_info->name().c_str());
t1.write_xy1_to_cell(2, 2, dx, dy, 0.0, 0.0, eng.conf_info.obs_info->name().c_str());

   //
   // Level Name
   //

nextline();

t1.write_xy1_to_cell(3, 1, dx, dy, 0.0, 0.0, eng.conf_info.fcst_info->level_name().c_str());
t1.write_xy1_to_cell(3, 2, dx, dy, 0.0, 0.0, eng.conf_info.obs_info->level_name().c_str());

   //
   // Units
   //

nextline();

t1.write_xy1_to_cell(4, 1, dx, dy, 0.0, 0.0, eng.conf_info.fcst_info->units().c_str());
t1.write_xy1_to_cell(4, 2, dx, dy, 0.0, 0.0, eng.conf_info.obs_info->units().c_str());

   //
   // Initialization Time
   //

t = eng.fcst_raw->data.valid() - eng.fcst_raw->data.lead();

unix_to_mdyhms(t, mon, day, yr, hr, minute, sec);

snprintf(junk, sizeof(junk), "%04d %02d %02d", yr, mon, day);

t1.write_xy1_to_cell(5, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%02d:%.02d:%02d", hr, minute, sec);

t1.write_xy1_to_cell(6, 1, dx, dy, 0.0, 0.0, junk);

t = eng.obs_raw->data.valid() - eng.obs_raw->data.lead();

unix_to_mdyhms(t, mon, day, yr, hr, minute, sec);

snprintf(junk, sizeof(junk), "%04d %02d %02d", yr, mon, day);
t1.write_xy1_to_cell(5, 2, dx, dy, 0.0, 0.0, junk);
snprintf(junk, sizeof(junk), "%02d:%02d:%02d", hr, minute, sec);
t1.write_xy1_to_cell(6, 2, dx, dy, 0.0, 0.0, junk);
text_y -= 2.0*TextSep;

   //
   // Valid time
   //

t = eng.fcst_raw->data.valid();

unix_to_mdyhms(t, mon, day, yr, hr, minute, sec);

snprintf(junk, sizeof(junk), "%04d %02d %02d", yr, mon, day);

t1.write_xy1_to_cell(7, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%02d:%02d:%02d", hr, minute, sec);

t1.write_xy1_to_cell(8, 1, dx, dy, 0.0, 0.0, junk);

t = eng.obs_raw->data.valid();

unix_to_mdyhms(t, mon, day, yr, hr, minute, sec);

snprintf(junk, sizeof(junk), "%04d %02d %02d", yr, mon, day);

t1.write_xy1_to_cell(7, 2, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%02d:%02d:%02d", hr, minute, sec);

t1.write_xy1_to_cell(8, 2, dx, dy, 0.0, 0.0, junk);

text_y -= 2.0*TextSep;

   //
   // Accumulation time
   //

strcpy(junk, sec_to_hhmmss_colon(eng.fcst_raw->data.accum()).c_str());
t1.write_xy1_to_cell(9, 1, dx, dy, 0.0, 0.0, junk);

strcpy(junk, sec_to_hhmmss_colon(eng.obs_raw->data.accum()).c_str());
t1.write_xy1_to_cell(9, 2, dx, dy, 0.0, 0.0, junk);
nextline();

   ////////////////////////////////////////////////////////////////////
   //
   // ModeFuzzyEngine Weights
   //
   ////////////////////////////////////////////////////////////////////

choose_font(31, 11.0);

nextline();

Htab_a = Htab_1 - 0.5*View_box.width();
Htab_b = Htab_a + 8.0*TextSep;
Htab_c = Htab_a + 11.0*TextSep;

TableHelper t2;

t2.set(*this, 6, 3);

for (j=0; j<(t2.nrows()); ++j)  t2.set_row_height(j, 15.0);

t2.set_col_width(0, 135.0);
t2.set_col_width(1, 0.5*(t1.width() - t2.col_width(0)));
t2.set_col_width(2, t2.col_width(1));

t2.set_pin(t1.left(), t1.bottom() - 15.0, 0.0, 1.0);

r = 1;

for (j=0; j<(t2.nrows()); ++j)  {

   if ( j%2 )  t2.fill_cell(j, 0, blue2);
   else        t2.fill_cell(j, 0, blue1);

}

t2.fill_cell(r, 1, light_gray);  t2.fill_cell(r, 2, light_gray);  r+=2;
t2.fill_cell(r, 1, light_gray);  t2.fill_cell(r, 2, light_gray);  r+=2;

r = 0;

line(t2.left(), t2.row_bottom(r), t2.right(), t2.row_bottom(r));  ++r;
line(t2.left(), t2.row_bottom(r), t2.right(), t2.row_bottom(r));  ++r;
line(t2.left(), t2.row_bottom(r), t2.right(), t2.row_bottom(r));  ++r;
line(t2.left(), t2.row_bottom(r), t2.right(), t2.row_bottom(r));  ++r;
line(t2.left(), t2.row_bottom(r), t2.right(), t2.row_bottom(r));  ++r;

c = 0;

line(t2.col_right(c), t2.top(), t2.col_right(c), t2.bottom());  ++c;

line(t2.col_right(c), t2.top(), t2.col_right(c), t2.row_top(t2.nrows() - 1));  ++c;

t2.outline_table(0.2, black);

bold();

r = 0;

c = 0;

t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Centroid/Boundary");
t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Convex Hull/Angle");
t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Aspect/Area");
t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Int Area/Curvature");
t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Complexity/Intensity");
t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Total Interest Thresh");

   //
   // Centroid and Boundary Distance Weights
   //

roman();

r = 0;

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.centroid_dist_wt);
t2.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.boundary_dist_wt);
t2.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);
++r;
nextline();

   //
   // Convex Hull Distance and Angle Difference Weights
   //

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.convex_hull_dist_wt);
t2.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.angle_diff_wt);
t2.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);
++r;
nextline();

   //
   // Aspect Ratio Difference and Area Ratio Weights
   //

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.aspect_diff_wt);
t2.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.area_ratio_wt);
t2.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);
++r;
nextline();

   //
   // Intesection Over Minimum Area Weights and Curvature Ratio Weights
   //

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.int_area_ratio_wt);
t2.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.curvature_ratio_wt);
t2.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);
++r;
nextline();

   //
   // Complexity Ratio and Intensity Ratio Weights
   //

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.complexity_ratio_wt);
t2.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.inten_perc_ratio_wt);
t2.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);
++r;
nextline();

   //
   // Total Interest Threshold
   //

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.total_interest_thresh);

t2.write_xy1_to_cell(r, 2, 0.0, dy, 0.5, 0.0, junk);

nextline();

   ////////////////////////////////////////////////////////////////////
   //
   // ModeFuzzyEngine configuration
   //
   ////////////////////////////////////////////////////////////////////

choose_font(31, 11.0);

text_y = Vtab_3 - 1.0*TextSep;
Htab_a = Htab_2 - 0.5*View_box.width();
Htab_b = Htab_a + 5.0*TextSep;
Htab_c = Htab_a + 10.0*TextSep;

if ( eng_type == FOEng )  do_page_1_FOEng (eng, eng_type, title);
else                      do_page_1_other (eng, eng_type, title);

showpage();

return;

}


////////////////////////////////////////////////////////////////////////


void ModePsFile::do_page_1_FOEng(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

int j, r;
char junk[1024];
ConcatString label, thresh_str;
ConcatString tmp1_str, tmp2_str, tmp3_str;
double v;
double x, y;
TableHelper t;
const double w = 240.0;

t.set(*this, 15, 3);

for (j=0; j<(t.nrows()); ++j)  t.set_row_height(j, 15.0);

t.set_col_width(0, 100.0);
t.set_col_width(1, 0.5*(w - t.col_width(0)));
t.set_col_width(2, t.col_width(1));

t.set_pin(Htab_a, Vtab_3 - 2.0, 0.0, 1.0);

for (j=1; j<(t.nrows()); ++j)  {

   if ( j%2 )  t.fill_cell(j, 0, blue1);
   else        t.fill_cell(j, 0, blue2);

}

t.fill_cell(0, 1, light_green);
t.fill_cell(0, 2, light_green);

t.fill_cell(0, 0, dark_gray);

for (j=2; j<(t.nrows()); j+=2)  {

   t.fill_cell(j, 1, light_gray);
   t.fill_cell(j, 2, light_gray);

}


for (j=0; j<(t.nrows() - 1); ++j)  {

   y = t.row_bottom(j);

   line(t.left(), y, t.right(), y);

}

x = t.col_right(0);

line(x, t.top(), x, t.bottom());

x = t.col_right(1);

line(x, t.top(), x, t.row_bottom(4));

line(x, t.row_bottom(5), x, t.row_bottom(7));

line(x, t.row_bottom(8), x, t.row_bottom(13));

t.outline_table(0.2, black);

bold();

t.write_xy1_to_cell(0, 1, dx, dy, 0.0, 0.0, FcstString.c_str());
t.write_xy1_to_cell(0, 2, dx, dy, 0.0, 0.0,  ObsString.c_str());

j = 1;

t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Mask M/G/P");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Conv Radius");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Conv Thresh");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Obj Filters");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Inten Perc");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Merge Thresh");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Merging");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Matching");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Simple/M/U");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Area");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Area M/U");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Cluster");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "MMI");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "MMI (F+O)");

   //
   // Field name
   //

nextline();

r = 1;

roman();

      //
      // Mask missing, grid, and polyline Flags
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Mask M/G/P:");
      if(eng.conf_info.mask_missing_flag == FieldType_Both ||
         eng.conf_info.mask_missing_flag == FieldType_Fcst) tmp1_str = "on";
      else                                                  tmp1_str = "off";
      if(eng.conf_info.mask_grid_flag == FieldType_Both ||
         eng.conf_info.mask_grid_flag == FieldType_Fcst)    tmp2_str = "on";
      else                                                  tmp2_str = "off";
      if(eng.conf_info.mask_grid_flag == FieldType_Both ||
         eng.conf_info.mask_grid_flag == FieldType_Fcst)    tmp3_str = "on";
      else                                                  tmp3_str = "off";
      label << cs_erase << tmp1_str << '/' << tmp2_str << '/' << tmp3_str;
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, label.c_str());

      if(eng.conf_info.mask_missing_flag == FieldType_Both ||
         eng.conf_info.mask_missing_flag == FieldType_Obs)  tmp1_str = "on";
      else                                                  tmp1_str = "off";
      if(eng.conf_info.mask_grid_flag == FieldType_Both ||
         eng.conf_info.mask_grid_flag == FieldType_Obs)     tmp2_str = "on";
      else                                                  tmp2_str = "off";
      if(eng.conf_info.mask_grid_flag == FieldType_Both ||
         eng.conf_info.mask_grid_flag == FieldType_Obs)     tmp3_str = "on";
      else                                                  tmp3_str = "off";
      label << cs_erase << tmp1_str << '/' << tmp2_str << '/' << tmp3_str;
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, label.c_str());

      ++r;
      nextline();

      //
      // Convolution Radius
      //
      snprintf(junk, sizeof(junk), "%d", eng.conf_info.fcst_conv_radius);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%d", eng.conf_info.obs_conv_radius);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Convolution Threshold
      //
      thresh_str = eng.conf_info.fcst_conv_thresh.get_str(2);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, thresh_str.c_str());

      thresh_str = eng.conf_info.obs_conv_thresh.get_str(2);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, thresh_str.c_str());

      ++r;
      nextline();

      //
      // Object Filters
      //

      snprintf(junk, sizeof(junk), "%i", (int) eng.conf_info.fcst_filter_attr_map.size());
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%i", (int) eng.conf_info.obs_filter_attr_map.size());
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Intensity Percentile
      //

           if(nint(eng.conf_info.inten_perc_value) == 101) label = "mean";
      else if(nint(eng.conf_info.inten_perc_value) == 102) label = "sum";
      else {
         snprintf(junk, sizeof(junk), "p%d", eng.conf_info.inten_perc_value);
         label = junk;
      }
      t.write_xy1_to_cell(r, 2, 0.0, dy, 0.5, 0.0, label.c_str());

      ++r;
      nextline();

      //
      // Merge Threshold
      //

      thresh_str = eng.conf_info.fcst_merge_thresh.get_str(2);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, thresh_str.c_str());

      thresh_str = eng.conf_info.obs_merge_thresh.get_str(2);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, thresh_str.c_str());

      ++r;
      nextline();

      /////////////////////////////////////////////////////////////////
      //
      // Matching/Merging Criteria
      //
      /////////////////////////////////////////////////////////////////

      //
      // Merging flag
      //

           if(eng.conf_info.fcst_merge_flag == MergeType_Thresh) label = "thresh";
      else if(eng.conf_info.fcst_merge_flag == MergeType_Engine) label = "engine";
      else if(eng.conf_info.fcst_merge_flag == MergeType_Both)   label = "thresh/engine";
      else                                                       label = "none";
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, label.c_str());

           if(eng.conf_info.obs_merge_flag == MergeType_Thresh) label = "thresh";
      else if(eng.conf_info.obs_merge_flag == MergeType_Engine) label = "engine";
      else if(eng.conf_info.obs_merge_flag == MergeType_Both)   label = "thresh/engine";
      else                                                      label = "none";
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, label.c_str());

      ++r;
      nextline();

      //
      // Matching scheme
      //

           if(eng.conf_info.match_flag == MatchType_MergeBoth) label = "match/merge";
      else if(eng.conf_info.match_flag == MatchType_MergeFcst) label = "match/fcst merge";
      else if(eng.conf_info.match_flag == MatchType_NoMerge)   label = "match/no merge";
      else                                                     label = "none";
      t.write_xy1_to_cell(r, 2, 0.0, dy, 0.5, 0.0, label.c_str());

      ++r;
      nextline();

      /////////////////////////////////////////////////////////////////
      //
      // Object Counts and Areas
      //
      /////////////////////////////////////////////////////////////////

      //
      // Simple objects counts (Matched Simples/Unmatched Simples)
      //

      snprintf(junk, sizeof(junk), "%i/%i/%i", eng.n_fcst,
              eng.get_matched_fcst(0), eng.get_unmatched_fcst(0));
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%i/%i/%i", eng.n_obs,
              eng.get_matched_obs(0), eng.get_unmatched_obs(0));
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Area counts
      //

      snprintf(junk, sizeof(junk), "%d", eng.get_matched_fcst(1) + eng.get_unmatched_fcst(1));
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%d", eng.get_matched_obs(1) + eng.get_unmatched_obs(1));
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Area counts (Matched Simple/Unmatched Simples)
      //

      snprintf(junk, sizeof(junk), "%i/%i",
              eng.get_matched_fcst(1),  eng.get_unmatched_fcst(1));
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%i/%i",
              eng.get_matched_obs(1),  eng.get_unmatched_obs(1));
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Cluster object counts
      //

      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      /////////////////////////////////////////////////////////////////
      //
      // Interest Values
      //
      /////////////////////////////////////////////////////////////////

      //
      // Median of Maximum Interest Values
      //

      v = interest_percentile(eng, 50.0, 1);
      snprintf(junk, sizeof(junk), "%.4f", v);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      v = interest_percentile(eng, 50.0, 2);
      snprintf(junk, sizeof(junk), "%.4f", v);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Median of Maximum Interest Values
      //

      v = interest_percentile(eng, 50.0, 3);
      snprintf(junk, sizeof(junk), "%.4f", v);
      t.write_xy1_to_cell(r, 2, 0.0, dy, 0.5, 0.0, junk);

      ++r;
      nextline();

   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////

void ModePsFile::do_page_1_other(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

int j, c;
char junk[1024];
TableHelper t;
const int simple_row  = 1;
const int cluster_row = 2;
const int area_row    = 3;
const int fcst_col    = 1;
const int  obs_col    = 2;

t.set(*this, 4, 3);

for (j=0; j<(t.nrows()); ++j)  t.set_row_height(j, 15.0);

t.set_col_width(0, 50.0);
t.set_col_width(1, 60.0);
t.set_col_width(2, 60.0);

t.set_pin(Htab_a, table_bottom, 0.0, 0.0);

t.fill_row(0, light_green);
t.fill_col(0, blue1);
t.fill_cell(0, 0, dark_gray);

t.draw_skeleton(0.2);
t.outline_table(0.2, black);



      text_y -= 12.0*TextSep;

      //
      // Field name
      //
      t.write_xy1_to_cell(0, fcst_col, 0.5*(t.col_width(fcst_col)), dy, 0.5, 0.0, FcstString.c_str());
      t.write_xy1_to_cell(0,  obs_col, 0.5*(t.col_width(fcst_col)), dy, 0.5, 0.0,  ObsString.c_str());
      nextline();

      /////////////////////////////////////////////////////////////////
      //
      // Object Counts and Areas
      //
      /////////////////////////////////////////////////////////////////

      //
      // Simple objects counts
      //

      bold();
      t.write_xy1_to_cell(simple_row,  0, dx, dy, 0.0, 0.0, "Simple");
      t.write_xy1_to_cell(cluster_row, 0, dx, dy, 0.0, 0.0, "Cluster");
      t.write_xy1_to_cell(area_row,    0, dx, dy, 0.0, 0.0, "Area");
      roman();
      snprintf(junk, sizeof(junk), "%d", eng.n_fcst);
      c = fcst_col;
      t.write_xy1_to_cell(simple_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%d", eng.n_obs);
      c = obs_col;
      t.write_xy1_to_cell(simple_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      nextline();

      //
      // Area counts
      //

      snprintf(junk, sizeof(junk), "%d", eng.get_matched_fcst(1) + eng.get_unmatched_fcst(1));
      c = fcst_col;
      t.write_xy1_to_cell(area_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%d", eng.get_matched_obs(1) + eng.get_unmatched_obs(1));
      c = obs_col;
      t.write_xy1_to_cell(area_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      nextline();

      //
      // Cluster object counts
      //

      snprintf(junk, sizeof(junk), "%d", eng.collection.n_sets);
      c = fcst_col;
      t.write_xy1_to_cell(cluster_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%d", eng.collection.n_sets);
      c = obs_col;
      t.write_xy1_to_cell(cluster_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      nextline();

   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////
