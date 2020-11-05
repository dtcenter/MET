// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include "plot_point_obs_conf_info.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class PlotPointObsOpt
//
////////////////////////////////////////////////////////////////////////

PlotPointObsOpt::PlotPointObsOpt() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PlotPointObsOpt::~PlotPointObsOpt() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsOpt::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsOpt::clear() {

   // Initialize values
// JHG, work here
   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsOpt::process_config(Dictionary &dict) {

   // Conf: JHG work here

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class PlotPointObsConfInfo
//
////////////////////////////////////////////////////////////////////////

PlotPointObsConfInfo::PlotPointObsConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PlotPointObsConfInfo::~PlotPointObsConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::init_from_scratch() {

   // Initialize pointers
   data_plane_info = (VarInfo *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::clear() {

   // Initialize values
   data_plane_flag = false;
   data_ctable.clear();
   data_ctable_flag = false;
   data_colorbar_flag = false;
   conf.clear();
   plot_opts.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::read_config(const char *user_file_name) {

   // Read the config file constants and map data
   conf.read(replace_path(config_const_filename).c_str());
   conf.read(replace_path(config_map_data_filename).c_str());
   conf.read(replace_path(default_config_filename).c_str());

   // Read the user file name, if provided
   if(user_file_name) conf.read(user_file_name);

   return;
}

////////////////////////////////////////////////////////////////////////

void PlotPointObsConfInfo::process_config() {
   ConcatString s;
   StringArray sa;
   Dictionary *dict = (Dictionary *) 0;

   // Dump the contents of the config file
   if(mlog.verbosity_level() >= 5) conf.dump(cout);

   // Initialize
   clear();

   // Conf: JHG work here

   return;
}

////////////////////////////////////////////////////////////////////////
