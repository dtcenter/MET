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


void ModePsFile::do_fcst_enlarge_page(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

double Vtab;
const double Htab_cen = PageWidth/2.0;

   /////////////////////////////////////////////////////////////////
   //
   // New Page: englargement of forecast fields
   //
   /////////////////////////////////////////////////////////////////

inc_pagenumber();

choose_font(31, 24.0);
write_centered_text(1, 1, Htab_cen, 752.0, 0.5, 0.5, title);
write_centered_text(1, 1, Htab_cen, 722.0, 0.5, 0.5, FcstString.c_str());

   /////////////////////////////////////////////////////////////////
   //
   // Draw raw forecast field
   //
   /////////////////////////////////////////////////////////////////

Vtab = PageHeight - 4.0*Vmargin;
set_view(Vtab - LargePlotHeight, Vtab, Htab_cen);
comment("fcst enlarge page: fcst raw");
render_ppm(eng, eng_type, *(eng.fcst_raw), 1, 0);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_colorbar(true);

   /////////////////////////////////////////////////////////////////
   //
   // Draw fcst split field
   //
   /////////////////////////////////////////////////////////////////

Vtab -= LargePlotHeight;
set_view(Vtab - LargePlotHeight, Vtab, Htab_cen);
comment("fcst enlarge page: fcst split");
render_ppm(eng, eng_type, *(eng.fcst_split), 1, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_convex_hulls(eng, 1, 0);

   /////////////////////////////////////////////////////////////////
   //
   // Finished with this page
   //
   /////////////////////////////////////////////////////////////////

showpage();

return;

}
