// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <cmath>

#include "tc_gen_conf_info.h"

#include "apply_mask.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCGenVxOpt
//
////////////////////////////////////////////////////////////////////////

TCGenVxOpt::TCGenVxOpt() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCGenVxOpt::~TCGenVxOpt() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void TCGenVxOpt::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenVxOpt::clear() {

   Desc.clear();
   AModel.clear();
   BModel.clear();
   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();
   ValidBeg = ValidEnd = (unixtime) 0;
   VxMaskName.clear();
   VxPolyMask.clear();
   VxGridMask.clear();
   VxAreaMask.clear();
   DLandThresh.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenVxOpt::process_config(Dictionary &dict) {
   int i, j;
   ConcatString file_name;

   // Conf: Desc
   Desc = parse_conf_string(&Conf, conf_key_desc);

   // Conf: AModel and BModel
   AModel = Conf.lookup_string_array(conf_key_amodel);
   BModel = Conf.lookup_string_array(conf_key_bmodel);

   // Conf: StormId
   StormId = Conf.lookup_string_array(conf_key_storm_id);

   // Conf: Basin
   Basin = Conf.lookup_string_array(conf_key_basin);

   // Conf: Cyclone
   Cyclone = Conf.lookup_string_array(conf_key_cyclone);

   // Conf: StormName
   StormName = Conf.lookup_string_array(conf_key_storm_name);

   // Conf: ValidBeg, ValidEnd
   ValidBeg = Conf.lookup_unixtime(conf_key_valid_beg);
   ValidEnd = Conf.lookup_unixtime(conf_key_valid_end);

   // Conf: VxMask
   if(nonempty(Conf.lookup_string(conf_key_vx_mask).c_str())) {
      file_name = replace_path(Conf.lookup_string(conf_key_vx_mask));
      mlog << Debug(2) << "Masking File: " << file_name << "\n";
      parse_poly_mask(file_name, VxPolyMask, VxGridMask, VxAreaMask,
                      VxMaskName);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class TCGenConfInfo
//
////////////////////////////////////////////////////////////////////////

TCGenConfInfo::TCGenConfInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCGenConfInfo::~TCGenConfInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void TCGenConfInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenConfInfo::clear() {

   for(size_t i=0; i<VxOpt.size(); i++) VxOpt[i].clear();
   BestTechnique.clear();
   DLandGrid.clear();
   DLandData.clear();
   Version.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenConfInfo::read_config(const char *default_file_name,
                                const char *user_file_name) {

   // Read the config file constants
   Conf.read(replace_path(config_const_filename).c_str());

   // Read the default config file
   Conf.read(default_file_name);

   // Read the user-specified config file
   Conf.read(user_file_name);

   // Process the configuration file
   process_config();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenConfInfo::process_config() {
   Dictionary *dict = (Dictionary *) 0;

   // Conf: BestTechnique
   BestTechnique = Conf.lookup_string_array(conf_key_best_technique);
   BestTechnique.set_ignore_case(true);

   // Conf: Version
   Version = Conf.lookup_string(conf_key_version);
   check_met_version(Version.c_str());

   // Conf: Filter



   return;
}

////////////////////////////////////////////////////////////////////////

void TCGenConfInfo::load_dland() {

   // JHG, read DLand file and store in DLandGrid and DLandData

   // Conf: DLandFile
   DLandFile = Conf.lookup_string(conf_key_dland_file);

   return;
}

////////////////////////////////////////////////////////////////////////

double TCGenConfInfo::get_dland(double lat, double lon) {

   if(DLandGrid.nxy() == 0) load_dland();

   // JHG compute the distance to land and return it

   return(0.0);
}

////////////////////////////////////////////////////////////////////////
