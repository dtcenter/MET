

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "mode_ps_file.h"


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
write_centered_text(1, 1, Htab_a, 727.0, 0.5, 0.5, FcstString);
write_centered_text(1, 1, Htab_b, 727.0, 0.5, 0.5, ObsString);

   /////////////////////////////////////////////////////////////////
   //
   // Draw fcst split field
   //
   /////////////////////////////////////////////////////////////////

set_view(Vtab_1, Vtab_1 + SmallPlotHeight, Htab_a);
render_image(eng, eng_type, *(eng.fcst_split), 1, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 1, 0);

   /////////////////////////////////////////////////////////////////
   //
   // Draw obs split field
   //
   /////////////////////////////////////////////////////////////////

set_view(Vtab_1, Vtab_1 + SmallPlotHeight, Htab_b);
render_image(eng, eng_type, *(eng.obs_split), 0, 1);
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

choose_font(31, 11.0);

Vtab = Vtab_2 - 1.0*TextSep;
Htab = Hmargin;

dx = 3.0*TextSep;
dy = 10.0;

   //
   // Plot the column headers
   //

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "CLUS");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "PAIR");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "CEN");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "DIST");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "ANG");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "DIFF");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "FCST");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "AREA");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "OBS");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "AREA");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "INTER");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "AREA");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "UNION");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "AREA");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "SYM");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "DIFF");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "FCST");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "INT50");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "OBS");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "INT50");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "FCST");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "INT90");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "OBS");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "INT90");
Htab += dx;

write_centered_text(1, 1, Htab, Vtab,      0.0, 0.5, "TOT");
write_centered_text(1, 1, Htab, Vtab - dy, 0.0, 0.5, "INTR");
Htab += dx;

Vtab -= (TextSep + 10.0);

   //
   //
   //

dx = 3.0*TextSep;

for(i=0; i<eng.n_clus && Vtab >= Vmargin; i++) {

   Htab = Hmargin;

   // Cluster ID
   snprintf(junk, sizeof(junk), "%i", i+1);
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Centroid Distance
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].centroid_dist);
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Angle Difference
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].angle_diff);
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Forecast Area
   snprintf(junk, sizeof(junk), "%i", nint(eng.pair_clus[i].Fcst[0].area));
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Observation Area
   snprintf(junk, sizeof(junk), "%i", nint(eng.pair_clus[i].Obs[0].area));
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Intersection Area
   snprintf(junk, sizeof(junk), "%i", nint(eng.pair_clus[i].intersection_area));
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Union Area
   snprintf(junk, sizeof(junk), "%i", nint(eng.pair_clus[i].union_area));
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Symmetric Difference
   snprintf(junk, sizeof(junk), "%i", nint(eng.pair_clus[i].symmetric_diff));
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Forecast median intensity
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].Fcst[0].intensity_ptile.p50);
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Observation median intensity
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].Obs[0].intensity_ptile.p50);
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Forecast 90th percentile of intensity
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].Fcst[0].intensity_ptile.p90);
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Observation median intensity
   snprintf(junk, sizeof(junk), "%.2f", eng.pair_clus[i].Obs[0].intensity_ptile.p90);
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, junk);
   Htab += dx;

   // Total Interest
   if(eng.info_clus[i].interest_value < 0) label = na_str;
   else {
      snprintf(junk, sizeof(junk), "%.4f", eng.info_clus[i].interest_value);
      label = junk;
   }
   write_centered_text(1, 1, Htab, Vtab, 0.0, 0.5, label);
   Htab += dx;

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




