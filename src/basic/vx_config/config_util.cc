// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include "config_util.h"

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_version(MetConfig *conf) {
   ConcatString s;

   s = conf->lookup_string(conf_version);

   check_met_version(s);

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString parse_conf_model(MetConfig *conf) {
   ConcatString s;

   s = conf->lookup_string(conf_model);

   // Check that it's non-empty and contains no whitespace
   if(s.empty() || check_reg_exp(ws_reg_exp, s) == true) {
      mlog << Error << "\nparse_conf_model() -> "
           << "The model name (\"" << s << "\") must be non-empty and "
           << "contain no embedded whitespace.\n\n";
      exit(1);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////
