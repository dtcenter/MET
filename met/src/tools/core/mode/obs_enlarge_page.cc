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


////////////////////////////////////////////////////////////////////////

void ModePsFile::do_obs_enlarge_page(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

double Vtab;
const double Htab_cen = PageWidth/2.0;

   /////////////////////////////////////////////////////////////////
   //
   // New Page: englargement of observation fields
   //
   /////////////////////////////////////////////////////////////////

inc_pagenumber();

choose_font(31, 24.0);
write_centered_text(1, 1, Htab_cen, 752.0, 0.5, 0.5, title);
write_centered_text(1, 1, Htab_cen, 722.0, 0.5, 0.5, ObsString.c_str());

   /////////////////////////////////////////////////////////////////
   //
   // Draw raw observation field
   //
   /////////////////////////////////////////////////////////////////

Vtab = PageHeight - 4.0*Vmargin;
set_view(Vtab - LargePlotHeight, Vtab, Htab_cen);
comment("obs enlarge page: obs raw");
render_ppm(eng, eng_type, *(eng.obs_raw), 0, 0);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_colorbar(false);

   /////////////////////////////////////////////////////////////////
   //
   // Draw observation split field
   //
   /////////////////////////////////////////////////////////////////

Vtab -= LargePlotHeight;
set_view(Vtab - LargePlotHeight, Vtab, Htab_cen);
comment("obs enlarge page: obs split");
render_ppm(eng, eng_type, *(eng.obs_split), 0, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 0, 0);

   /////////////////////////////////////////////////////////////////
   //
   // Finished with this page
   //
   /////////////////////////////////////////////////////////////////

showpage();

return;

}
