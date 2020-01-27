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


void ModePsFile::do_overlap_page(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

double Vtab;
const double Htab_cen = PageWidth/2.0;
ConcatString tmp_str;

   /////////////////////////////////////////////////////////////////
   //
   // New Page: overlap of object fields
   //
   /////////////////////////////////////////////////////////////////

inc_pagenumber();

   /////////////////////////////////////////////////////////////////
   //
   // Draw the forecast object field and observation boundaries
   //
   /////////////////////////////////////////////////////////////////

Vtab = PageHeight - 2.0*Vmargin;
set_view(Vtab - LargePlotHeight, Vtab, Htab_cen);

tmp_str << cs_erase << FcstString << " Objects with " << ObsString << " Outlines";
choose_font(31, 24.0);
write_centered_text(1, 1, Htab_cen, View_box.top() + TextSep/2.0,
                      0.5, 0.5, tmp_str.c_str());

comment("overlap page: fcst objects with obs boundaries");
render_ppm(eng, eng_type, *(eng.fcst_split), 1, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_boundaries(eng, 0);

   /////////////////////////////////////////////////////////////////
   //
   // Draw the observation object field and forecast boundaries
   //
   /////////////////////////////////////////////////////////////////

Vtab = Vtab - LargePlotHeight - 2.0*Vmargin;
set_view(Vtab - LargePlotHeight, Vtab, Htab_cen);

tmp_str << cs_erase << ObsString << " Objects with " << FcstString << " Outlines";
choose_font(31, 24.0);
write_centered_text(1, 1, Htab_cen, View_box.top() + TextSep/2.0,
                      0.5, 0.5, tmp_str.c_str());

comment("overlap page: obs objects with fcst boundaries");
render_ppm(eng, eng_type, *(eng.obs_split), 0, 1);
outline_view();
draw_map( &(eng.conf_info.conf) );
draw_boundaries(eng, 1);

   /////////////////////////////////////////////////////////////////
   //
   // Finished with this page
   //
   /////////////////////////////////////////////////////////////////

showpage();

return;

}


////////////////////////////////////////////////////////////////////////



