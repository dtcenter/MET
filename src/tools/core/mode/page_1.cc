// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

// static const int max_interest_rows = 45;
static const int max_interest_rows = 29;


////////////////////////////////////////////////////////////////////////


void ModePsFile::do_page_1(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

int j;
char junk[1024];
// bool drew_line;
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

// set_family(ff_Helvetica);
set_family(ff_Palatino);
// set_family(ff_NewCentury);

setlinecap(1);
setlinejoin(1);

choose_font(31, 24.0);

write_centered_text(1, 1, 306.0, 752.0, 0.5, 0.5, title);

choose_font(31, 18.0);

write_centered_text(1, 1, Htab_1, 727.0, 0.5, 0.5, FcstString);
write_centered_text(1, 1, Htab_2, 727.0, 0.5, 0.5, ObsString);

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

// write_centered_text(1, 1, Htab_3 + 1.5*TextSep, 727.0, 0.5, 0.5, FcstShortString);
// write_centered_text(1, 1, Htab_3 + 4.5*TextSep, 727.0, 0.5, 0.5, ObsShortString);
// write_centered_text(1, 1, Htab_3 + 7.5*TextSep, 727.0, 0.5, 0.5, "Interest");

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

// t0.set_row_height(9, 10.0);

t0.set_pin(Htab_3, Vtab_1 + SmallPlotHeight + TextSep, 0.0, 1.0);

t0.fill_row(0, light_green);


bad_count = 0;

for(i=0; i<n; i++) {

   if ( eng.info[i].interest_value < eng.conf_info.total_interest_thresh )  ++bad_count;

}

for (j=0; j<bad_count; ++j)  {

   t0.fill_row(t0.nrows() - 1 - j, light_gray);

}

t0.draw_skeleton(0.2);
t0.outline_table(0.2, black);

bold();

t0.write_xy1_to_cell(0, 0, dx, dy, 0.0, 0.0, FcstShortString);
t0.write_xy1_to_cell(0, 1, dx, dy, 0.0, 0.0,  ObsShortString);
t0.write_xy1_to_cell(0, 2, dx, dy, 0.0, 0.0,  "Interest");

roman();

x = 0.5*(t0.col_width(0));

for(i=0; i<n; i++) {

   snprintf(junk, sizeof(junk), "%d", eng.info[i].fcst_number);

   t0.write_xy1_to_cell(i + 1, 0, x, dy, 0.5, 0.0, junk);

   snprintf(junk, sizeof(junk), "%d", eng.info[i].obs_number);

   t0.write_xy1_to_cell(i + 1, 1, x, dy, 0.5, 0.0, junk);

   if ( eng.info[i].interest_value < 0 ) label = na_str;
   else {
      label.set_precision(4);
      label << cs_erase << eng.info[i].interest_value;
   }

   t0.write_xy1_to_cell(i + 1, 2, dx, dy, 0.0, 0.0, label);

}


/*
drew_line = false;

for(i=0; i<(eng.n_fcst*eng.n_obs) && text_y >= Vmargin; i++) {

   if ( (eng.info[i].interest_value < eng.conf_info.total_interest_thresh) && !drew_line )  {

      write_centered_text(1, 1, Htab_3 + 4.5*TextSep, text_y, 0.5, 0.5, "----------------------------------");

      drew_line = true;

      nextline();

   }

   label << cs_erase << eng.info[i].fcst_number;
   write_centered_text(1, 1, Htab_3 + 1.5*TextSep, text_y, 0.5, 0.5, label);

   label << cs_erase << eng.info[i].obs_number;
   write_centered_text(1, 1, Htab_3 + 4.5*TextSep, text_y, 0.5, 0.5, label);

   if ( eng.info[i].interest_value < 0 ) label = na_str;
   else {
      label.set_precision(4);
      label << cs_erase << eng.info[i].interest_value;
   }
   write_centered_text(1, 1, Htab_3 + 7.5*TextSep, text_y, 0.5, 0.5, label);

   nextline();

}   //  for i
*/

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

// t1.set_row_height(9, 10.0);

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
// t1.fill_cell(1, 2, dark_gray);

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

// t1.draw_skeleton(0.2);
t1.outline_table(0.2, black);

   //
   // Field name
   //
// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, FcstString);
// write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, ObsString);
nextline();

bold();

t1.write_xy1_to_cell(0, 1, dx, dy, 0.0, 0.0, FcstString);
t1.write_xy1_to_cell(0, 2, dx, dy, 0.0, 0.0,  ObsString);

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
// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Model:");
// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, eng.conf_info.model);
nextline();

// t1.write_xy1_to_cell(1, 1, dx, dy, 0.0, 0.0, eng.conf_info.model);
t1.write_xy1_to_cell(1, 2, 0.0, dy, 0.5, 0.0, eng.conf_info.model);

   //
   // Variable Name
   //
// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Field:");
// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, eng.conf_info.fcst_info->name());
// write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, eng.conf_info.obs_info->name());
nextline();

t1.write_xy1_to_cell(2, 1, dx, dy, 0.0, 0.0, eng.conf_info.fcst_info->name());
t1.write_xy1_to_cell(2, 2, dx, dy, 0.0, 0.0, eng.conf_info.obs_info->name());

   //
   // Level Name
   //
// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Level:");
// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, eng.conf_info.fcst_info->level_name());
// write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, eng.conf_info.obs_info->level_name());
nextline();

t1.write_xy1_to_cell(3, 1, dx, dy, 0.0, 0.0, eng.conf_info.fcst_info->level_name());
t1.write_xy1_to_cell(3, 2, dx, dy, 0.0, 0.0, eng.conf_info.obs_info->level_name());

   //
   // Units
   //
// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Units:");
// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, eng.conf_info.fcst_info->units());
// write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, eng.conf_info.obs_info->units());
nextline();

t1.write_xy1_to_cell(4, 1, dx, dy, 0.0, 0.0, eng.conf_info.fcst_info->units());
t1.write_xy1_to_cell(4, 2, dx, dy, 0.0, 0.0, eng.conf_info.obs_info->units());

   //
   // Initialization Time
   //
// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Initial:");

t = eng.fcst_raw->data.valid() - eng.fcst_raw->data.lead();

unix_to_mdyhms(t, mon, day, yr, hr, minute, sec);

// snprintf(junk, sizeof(junk), "%.4i%.2i%.2i", yr, mon, day);
snprintf(junk, sizeof(junk), "%04d %02d %02d", yr, mon, day);

// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
t1.write_xy1_to_cell(5, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%02d:%.02d:%02d", hr, minute, sec);

// write_centered_text(1, 1, Htab_b, text_y - TextSep, 0.0, 0.5, junk);

t1.write_xy1_to_cell(6, 1, dx, dy, 0.0, 0.0, junk);

t = eng.obs_raw->data.valid() - eng.obs_raw->data.lead();

unix_to_mdyhms(t, mon, day, yr, hr, minute, sec);

snprintf(junk, sizeof(junk), "%04d %02d %02d", yr, mon, day);
// write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
t1.write_xy1_to_cell(5, 2, dx, dy, 0.0, 0.0, junk);
snprintf(junk, sizeof(junk), "%02d:%02d:%02d", hr, minute, sec);
// write_centered_text(1, 1, Htab_c, text_y-TextSep, 0.0, 0.5, junk);
t1.write_xy1_to_cell(6, 2, dx, dy, 0.0, 0.0, junk);
text_y -= 2.0*TextSep;

   //
   // Valid time
   //
// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Valid:");

t = eng.fcst_raw->data.valid();

unix_to_mdyhms(t, mon, day, yr, hr, minute, sec);

snprintf(junk, sizeof(junk), "%04d %02d %02d", yr, mon, day);

// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);

t1.write_xy1_to_cell(7, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%02d:%02d:%02d", hr, minute, sec);
// write_centered_text(1, 1, Htab_b, text_y-TextSep, 0.0, 0.5, junk);

t1.write_xy1_to_cell(8, 1, dx, dy, 0.0, 0.0, junk);

t = eng.obs_raw->data.valid();

unix_to_mdyhms(t, mon, day, yr, hr, minute, sec);

snprintf(junk, sizeof(junk), "%04d %02d %02d", yr, mon, day);
// write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);


t1.write_xy1_to_cell(7, 2, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%02d:%02d:%02d", hr, minute, sec);
// write_centered_text(1, 1, Htab_c, text_y-TextSep, 0.0, 0.5, junk);

t1.write_xy1_to_cell(8, 2, dx, dy, 0.0, 0.0, junk);

text_y -= 2.0*TextSep;

   //
   // Accumulation time
   //

strcpy(junk, sec_to_hhmmss_colon(eng.fcst_raw->data.accum()));
t1.write_xy1_to_cell(9, 1, dx, dy, 0.0, 0.0, junk);

strcpy(junk, sec_to_hhmmss_colon(eng.obs_raw->data.accum()));
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

t2.set(*this, 5, 3);

for (j=0; j<(t2.nrows()); ++j)  t2.set_row_height(j, 15.0);

t2.set_col_width(0, 135.0);
t2.set_col_width(1, 0.5*(t1.width() - t2.col_width(0)));
t2.set_col_width(2, t2.col_width(1));

// t2.set_row_height(9, 10.0);

t2.set_pin(t1.left(), t1.bottom() - 15.0, 0.0, 1.0);

// t2.fill_col(0, blue1);

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

c = 0;

line(t2.col_right(c), t2.top(), t2.col_right(c), t2.bottom());  ++c;

line(t2.col_right(c), t2.top(), t2.col_right(c), t2.row_top(t2.nrows() - 1));  ++c;

// t2.draw_skeleton(0.2);
t2.outline_table(0.2, black);

bold();

r = 0;

c = 0;

t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Centroid/Boundary");
t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Convex Hull/Angle");
t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Area/Intersection Area");
t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Complexity/Intensity");
t2.write_xy1_to_cell(r++, c, dx, dy, 0.0, 0.0, "Total Interest Thresh");

   //
   // Centroid and Boundary Distance Weights
   //

roman();

r = 0;

// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Centroid/Boundary:");
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.centroid_dist_wt);
// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
t2.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.boundary_dist_wt);
// write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
t2.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);
++r;
nextline();

   //
   // Convex Hull Distance and Angle Difference Weights
   //
// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Convex Hull/Angle:");
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.convex_hull_dist_wt);
// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
t2.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.angle_diff_wt);
// write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
t2.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);
++r;
nextline();

   //
   // Area Ratio and Intesection Over Minimum Area Weights
   //
// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Area/Intersection Area:");
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.area_ratio_wt);
// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
t2.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.int_area_ratio_wt);
// write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
t2.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);
++r;
nextline();

   //
   // Complexity Ratio and Intensity Ratio Weights
   //
// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Complexity/Intensity:");
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.complexity_ratio_wt);
// write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
t2.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.inten_perc_ratio_wt);
// write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
t2.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);
++r;
nextline();

   //
   // Total Interest Threshold
   //
// write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Total Interest Thresh:");
snprintf(junk, sizeof(junk), "%.2f", eng.conf_info.total_interest_thresh);
// write_centered_text(1, 1, (Htab_b+Htab_c)/2.0, text_y, 0.0, 0.5, junk);

t2.write_xy1_to_cell(r, 2, 0.0, dy, 0.5, 0.0, junk);

nextline();

   ////////////////////////////////////////////////////////////////////
   //
   // ModeFuzzyEngine configuration
   //
   ////////////////////////////////////////////////////////////////////

choose_font(31, 11.0);

text_y   = Vtab_3 - 1.0*TextSep;
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

t.set(*this, 16, 3);

for (j=0; j<(t.nrows()); ++j)  t.set_row_height(j, 15.0);

t.set_col_width(0, 100.0);
t.set_col_width(1, 0.5*(w - t.col_width(0)));
t.set_col_width(2, t.col_width(1));

// t.set_row_height(9, 10.0);

t.set_pin(Htab_a, Vtab_3 - 2.0, 0.0, 1.0);

for (j=1; j<(t.nrows()); ++j)  {

   if ( j%2 )  t.fill_cell(j, 0, blue1);
   else        t.fill_cell(j, 0, blue2);

}

// t.fill_col(0, blue1);

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

line(x, t.top(), x, t.row_bottom(8));

line(x, t.row_bottom(9), x, t.row_bottom(14));


// t.draw_skeleton(0.2);
t.outline_table(0.2, black);

bold();

t.write_xy1_to_cell(0, 1, dx, dy, 0.0, 0.0, FcstString);
t.write_xy1_to_cell(0, 2, dx, dy, 0.0, 0.0,  ObsString);

j = 1;

t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Mask M/G/P");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Raw Thresh");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Conv Radius");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Conv Thresh");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Area Thresh");
t.write_xy1_to_cell(j++, 0, dx, dy, 0.0, 0.0, "Inten Thresh");
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
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, FcstString);
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, ObsString);
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
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, label);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, label);

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
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, label);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, label);

      ++r;
      nextline();

      //
      // Raw threshold
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Raw Thresh:");
      thresh_str = eng.conf_info.fcst_raw_thresh.get_str(2);
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, thresh_str);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, thresh_str);

      thresh_str = eng.conf_info.obs_raw_thresh.get_str(2);
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, thresh_str);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, thresh_str);

      ++r;
      nextline();

      //
      // Convolution Radius
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Conv Radius:");
      // snprintf(junk, sizeof(junk), "%d gs", eng.conf_info.fcst_conv_radius);
      snprintf(junk, sizeof(junk), "%d", eng.conf_info.fcst_conv_radius);
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      // snprintf(junk, sizeof(junk), "%d gs", eng.conf_info.obs_conv_radius);
      snprintf(junk, sizeof(junk), "%d", eng.conf_info.obs_conv_radius);
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Convolution Threshold
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Conv Thresh:");
      thresh_str = eng.conf_info.fcst_conv_thresh.get_str(2);
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, thresh_str);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, thresh_str);

      thresh_str = eng.conf_info.obs_conv_thresh.get_str(2);
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, thresh_str);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, thresh_str);

      ++r;
      nextline();

      //
      // Area Threshold
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Area Thresh:");
      thresh_str = eng.conf_info.fcst_area_thresh.get_str(0);
      // snprintf(junk, sizeof(junk), "%s gs", thresh_str.text());
      snprintf(junk, sizeof(junk), "%s", thresh_str.text());
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      thresh_str = eng.conf_info.obs_area_thresh.get_str(0);
      // snprintf(junk, sizeof(junk), "%s gs", thresh_str.text());
      snprintf(junk, sizeof(junk), "%s", thresh_str.text());
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Intensity Percentile and Threshold
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Inten Thresh:");

           if(nint(eng.conf_info.fcst_inten_perc_value) == 101) tmp1_str = "mean";
      else if(nint(eng.conf_info.fcst_inten_perc_value) == 102) tmp1_str = "sum";
      else {
         snprintf(junk, sizeof(junk), "p%d", eng.conf_info.fcst_inten_perc_value);
         tmp1_str = junk;
      }
      thresh_str = eng.conf_info.fcst_inten_perc_thresh.get_str(2);
      label << cs_erase << tmp1_str << thresh_str;
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, label);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, label);

           if(nint(eng.conf_info.obs_inten_perc_value) == 101) tmp1_str = "mean";
      else if(nint(eng.conf_info.obs_inten_perc_value) == 102) tmp1_str = "sum";
      else {
         snprintf(junk, sizeof(junk), "p%.0i", eng.conf_info.obs_inten_perc_value);
         tmp1_str = junk;
      }
      thresh_str = eng.conf_info.obs_inten_perc_thresh.get_str(2);
      label << cs_erase << tmp1_str << thresh_str;
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, label);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, label);

      ++r;
      nextline();

      //
      // Merge Threshold
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Merge Thresh:");
      thresh_str = eng.conf_info.fcst_merge_thresh.get_str(2);
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, thresh_str);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, thresh_str);

      thresh_str = eng.conf_info.obs_merge_thresh.get_str(2);
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, thresh_str);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, thresh_str);

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
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Merging:");
           if(eng.conf_info.fcst_merge_flag == MergeType_Thresh) label = "thresh";
      else if(eng.conf_info.fcst_merge_flag == MergeType_Engine) label = "engine";
      else if(eng.conf_info.fcst_merge_flag == MergeType_Both)   label = "thresh/engine";
      else                                                       label = "none";
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, label);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, label);

           if(eng.conf_info.obs_merge_flag == MergeType_Thresh) label = "thresh";
      else if(eng.conf_info.obs_merge_flag == MergeType_Engine) label = "engine";
      else if(eng.conf_info.obs_merge_flag == MergeType_Both)   label = "thresh/engine";
      else                                                      label = "none";
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, label);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, label);

      ++r;
      nextline();

      //
      // Matching scheme
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Matching:");
           if(eng.conf_info.match_flag == MatchType_MergeBoth) label = "match/merge";
      else if(eng.conf_info.match_flag == MatchType_MergeFcst) label = "match/fcst merge";
      else if(eng.conf_info.match_flag == MatchType_NoMerge)   label = "match/no merge";
      else                                                     label = "none";
      // write_centered_text(1, 1, (Htab_b + Htab_c)/2.0, text_y, 0.0, 0.5, label);
      t.write_xy1_to_cell(r, 2, 0.0, dy, 0.5, 0.0, label);

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
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Simple/M/U:");
      snprintf(junk, sizeof(junk), "%i/%i/%i", eng.n_fcst,
              eng.get_matched_fcst(0), eng.get_unmatched_fcst(0));
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%i/%i/%i", eng.n_obs,
              eng.get_matched_obs(0), eng.get_unmatched_obs(0));
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Area counts
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Area:");
      // snprintf(junk, sizeof(junk), "%i gs", eng.get_matched_fcst(1) + eng.get_unmatched_fcst(1));
      snprintf(junk, sizeof(junk), "%d", eng.get_matched_fcst(1) + eng.get_unmatched_fcst(1));
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      // snprintf(junk, sizeof(junk), "%i gs", eng.get_matched_obs(1) + eng.get_unmatched_obs(1));
      snprintf(junk, sizeof(junk), "%d", eng.get_matched_obs(1) + eng.get_unmatched_obs(1));
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Area counts (Matched Simple/Unmatched Simples)
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Area M/U:");
      snprintf(junk, sizeof(junk), "%i/%i",
              eng.get_matched_fcst(1),  eng.get_unmatched_fcst(1));
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%i/%i",
              eng.get_matched_obs(1),  eng.get_unmatched_obs(1));
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Cluster object counts
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Cluster:");
      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%i", eng.collection.n_sets);
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
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
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "MMI:");
      v = interest_percentile(eng, 50.0, 1);
      snprintf(junk, sizeof(junk), "%.4f", v);
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 1, dx, dy, 0.0, 0.0, junk);

      v = interest_percentile(eng, 50.0, 2);
      snprintf(junk, sizeof(junk), "%.4f", v);
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
      t.write_xy1_to_cell(r, 2, dx, dy, 0.0, 0.0, junk);

      ++r;
      nextline();

      //
      // Median of Maximum Interest Values
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "MMI (F+O):");
      v = interest_percentile(eng, 50.0, 3);
      snprintf(junk, sizeof(junk), "%.4f", v);
      // write_centered_text(1, 1, (Htab_b+Htab_c)/2.0, text_y, 0.0, 0.5, junk);
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
      t.write_xy1_to_cell(0, fcst_col, 0.5*(t.col_width(fcst_col)), dy, 0.5, 0.0, FcstString);
      t.write_xy1_to_cell(0,  obs_col, 0.5*(t.col_width(fcst_col)), dy, 0.5, 0.0,  ObsString);
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, FcstString);
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, ObsString);
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
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Simple:");
      snprintf(junk, sizeof(junk), "%d", eng.n_fcst);
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
      c = fcst_col;
      t.write_xy1_to_cell(simple_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%d", eng.n_obs);
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
      c = obs_col;
      t.write_xy1_to_cell(simple_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      nextline();

      //
      // Area counts
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Area:");
      snprintf(junk, sizeof(junk), "%d", eng.get_matched_fcst(1) + eng.get_unmatched_fcst(1));
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
      c = fcst_col;
      t.write_xy1_to_cell(area_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%d", eng.get_matched_obs(1) + eng.get_unmatched_obs(1));
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
      c = obs_col;
      t.write_xy1_to_cell(area_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      nextline();

      //
      // Cluster object counts
      //
      // write_centered_text(1, 1, Htab_a, text_y, 0.0, 0.5, "Cluster:");
      snprintf(junk, sizeof(junk), "%d", eng.collection.n_sets);
      // write_centered_text(1, 1, Htab_b, text_y, 0.0, 0.5, junk);
      c = fcst_col;
      t.write_xy1_to_cell(cluster_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      snprintf(junk, sizeof(junk), "%d", eng.collection.n_sets);
      // write_centered_text(1, 1, Htab_c, text_y, 0.0, 0.5, junk);
      c = obs_col;
      t.write_xy1_to_cell(cluster_row, c, t.col_width(c) - 5.0, dy, 1.0, 0.0, junk);

      nextline();

   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////



