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


////////////////////////////////////////////////////////////////////////


   //
   //  Flag value of 2 indicates that both fcst and obs should be enlarged
   //  Flag value of 1 indicates that only the fcst should be enlarged
   //  Flag value of 0 indicates that only the obs should be enlarged
   //


void ModePsFile::plot_engine(ModeFuzzyEngine & eng, EngineType eng_type, const char * title)

{

   //
   //  setup fcst & obs strings
   //

if ( eng_type == FOEng )  { // Plot forecast versus observation

   FcstString      = "Forecast";
   FcstShortString = "Fcst";
   ObsString       = "Observation";
   ObsShortString  = "Obs";

} else if ( eng_type == FFEng )  { // Plot forecast versus forecast

   FcstString      = "Forecast";
   FcstShortString = "Fcst";
   ObsString       = "Forecast";
   ObsShortString  = "Fcst";

} else if ( eng_type == OOEng )  { // Plot observation versus observation

   FcstString      = "Observation";
   FcstShortString = "Obs";
   ObsString       = "Observation";
   ObsShortString  = "Obs";

}

   //
   //  do pages
   //

do_page_1(eng, eng_type, title);

if ( (eng_type == FOEng) || (eng_type == FFEng) )   do_fcst_enlarge_page(eng, eng_type, title);

if ( (eng_type == FOEng) || (eng_type == OOEng) )   do_obs_enlarge_page(eng, eng_type, title);

if ( eng_type == FOEng )  {

   do_overlap_page(eng, eng_type, title);
   do_cluster_page(eng, eng_type, title);

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////




