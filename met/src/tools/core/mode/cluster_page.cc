// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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


static const int max_table_rows = 20;


////////////////////////////////////////////////////////////////////////


static void write_header_cell(TableHelper & , int col, const char * s1, const char * s2);


////////////////////////////////////////////////////////////////////////


void ModePsFile::do_cluster_page(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

int i;
double Htab_a, Htab_b;
double Htab, Vtab;
double dx, dy;
const double Htab_cen = PageWidth/2.0;
ConcatString label;
char junk[1024];

   /////////////////////////////////////////////////////////////////
   //
   // New Page: cluster object pairs
   //
   /////////////////////////////////////////////////////////////////

inc_pagenumber();

Htab_a = Htab_cen - SmallPane.width()/4.0;
Htab_b = Htab_cen + SmallPane.width()/4.0;

Vtab = PageHeight - 2.0*Vmargin;
set_view(Vtab - LargePlotHeight, Vtab, Htab_cen);

choose_font(31, 24.0);
write_centered_text(1, 1, Htab_cen, View_box.top() + TextSep/2.0,
                          0.5, 0.5, "Cluster Object Information");

choose_font(31, 18.0);
write_centered_text(1, 1, Htab_a, 727.0, 0.5, 0.5, FcstString.c_str());
write_centered_text(1, 1, Htab_b, 727.0, 0.5, 0.5, ObsString.c_str());

   /////////////////////////////////////////////////////////////////
   //
   // Draw fcst split field
   //
   /////////////////////////////////////////////////////////////////

set_view(Vtab_1, Vtab_1 + SmallPlotHeight, Htab_a);
comment("cluster page: fcst split");
render_ppm(eng, eng_type, *(eng.fcst_split), 1, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 1, 0);

   /////////////////////////////////////////////////////////////////
   //
   // Draw obs split field
   //
   /////////////////////////////////////////////////////////////////

set_view(Vtab_1, Vtab_1 + SmallPlotHeight, Htab_b);
comment("cluster page: obs split");
render_ppm(eng, eng_type, *(eng.obs_split), 0, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 0, 0);

   /////////////////////////////////////////////////////////////////
   //
   // Draw fcst cluster ids
   //
   /////////////////////////////////////////////////////////////////

set_view(Vtab_2, Vtab_2 + SmallPlotHeight, Htab_a);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 1, 1);

   /////////////////////////////////////////////////////////////////
   //
   // Draw obs cluster ids
   //
   /////////////////////////////////////////////////////////////////

set_view(Vtab_2, Vtab_2 + SmallPlotHeight, Htab_b);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 0, 1);

   /////////////////////////////////////////////////////////////////
   //
   // Plot cluster pair attributes
   //
   /////////////////////////////////////////////////////////////////

int j, r, c;
TableHelper t;

j = eng.n_clus;

if ( j > max_table_rows )  j = max_table_rows;

t.set(*this, j + 1, 13);


for (j=0; j<(t.ncols()); ++j)  t.set_col_width(j, 45.0);

t.set_row_height(0, 25.0);

for (j=1; j<(t.nrows()); ++j)  t.set_row_height(j, 15.0);


t.set_pin(306.0, Vtab_2 - TextSep, 0.5, 1.0);

t.fill_row(0, blue1);

for (j=2; j<(t.nrows()); j+=2)  t.fill_row(j, light_gray);

t.draw_skeleton(0.2);
t.outline_table(0.2, black);



choose_font(31, 11.0);

Vtab = Vtab_2 - 1.0*TextSep;

   //
   // Plot the column headers
   //

c = 0;

bold();

write_header_cell(t, c++,  "CLUS", "PAIR");
write_header_cell(t, c++,   "CEN", "DIST");
write_header_cell(t, c++,   "ANG", "DIFF");
write_header_cell(t, c++,  "FCST", "AREA");
write_header_cell(t, c++,   "OBS", "AREA");
write_header_cell(t, c++, "INTER", "AREA");
write_header_cell(t, c++, "UNION", "AREA");
write_header_cell(t, c++,  "SYMM", "DIFF");
write_header_cell(t, c++,  "FCST", "INT 50");
write_header_cell(t, c++,   "OBS", "INT 50");
write_header_cell(t, c++,  "FCST", "INT 90");
write_header_cell(t, c++,   "OBS", "INT 90");
write_header_cell(t, c++,   "TOT", "INTR");

roman();

Vtab -= (TextSep + 10.0);

   //
   //
   //

dx = 3.0*TextSep;

dy = 2.0;

for(i=0; i<eng.n_clus && Vtab >= Vmargin; i++) {

   r = i + 1;

   c = 0;

   if ( r >= t.nrows() )  break;

   Htab = Hmargin;

   // Cluster ID
   snprintf(junk, sizeof(junk), "%d", i+1);
   t.write_xy1_to_cell(r, c++, 10.0, dy, 0.5, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Centroid Distance
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_cluster[i].centroid_dist);
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Angle Difference
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_cluster[i].angle_diff);
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Forecast Area
   snprintf(junk, sizeof(junk), "%i", nint(eng.pair_cluster[i].Fcst[0].area));
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Observation Area
   snprintf(junk, sizeof(junk), "%i", nint(eng.pair_cluster[i].Obs[0].area));
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Intersection Area
   snprintf(junk, sizeof(junk), "%i", nint(eng.pair_cluster[i].intersection_area));
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Union Area
   snprintf(junk, sizeof(junk), "%i", nint(eng.pair_cluster[i].union_area));
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Symmetric Difference
   snprintf(junk, sizeof(junk), "%i", nint(eng.pair_cluster[i].symmetric_diff));
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Forecast median intensity
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_cluster[i].Fcst[0].intensity_ptile.p50);
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Observation median intensity
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_cluster[i].Obs[0].intensity_ptile.p50);
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Forecast 90th percentile of intensity
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_cluster[i].Fcst[0].intensity_ptile.p90);
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Observation median intensity
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_cluster[i].Obs[0].intensity_ptile.p90);
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Total Interest
   if(eng.info_clus[i].interest_value < 0) label = na_str;
   else {
      snprintf(junk, sizeof(junk), "%.4f", eng.info_clus[i].interest_value);
      label = junk;
   }
   t.write_xy1_to_cell(r, c++, 40.0, dy, 1.0, 0.0, junk);
   // write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, label);
   // Htab += dx;

   Vtab -= TextSep;

}   //  for i

   /////////////////////////////////////////////////////////////////
   //
   // Finished with this page
   //
   /////////////////////////////////////////////////////////////////

showpage();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void write_header_cell(TableHelper & t, int col, const char * s1, const char * s2)

{

double v1 = 15.0;
double v2 =  3.0;
const double x = 0.5*(t.col_width(col));

t.write_xy1_to_cell(0, col, x, v1, 0.5, 0.0, s1);
t.write_xy1_to_cell(0, col, x, v2, 0.5, 0.0, s2);

return;

}


////////////////////////////////////////////////////////////////////////




