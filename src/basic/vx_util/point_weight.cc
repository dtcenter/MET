// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#include "vx_util.h"

#include "config_file.h"
#include "config_constants.h"

#include "point_weight.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
// Code for PointWeightInfo struct
//
///////////////////////////////////////////////////////////////////////////////

PointWeightInfo::PointWeightInfo() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

PointWeightInfo::~PointWeightInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

PointWeightInfo::PointWeightInfo(const PointWeightInfo &m) {

   init_from_scratch();

   assign(m);
}

////////////////////////////////////////////////////////////////////////

PointWeightInfo & PointWeightInfo::operator=(const PointWeightInfo &m) noexcept {

   if(this == &m) return *this;

   assign(m);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void PointWeightInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PointWeightInfo::clear() {
   Type = PointWeightType::None;
   KDERefAngle = bad_data_double;
   WriteWeights = false;
   SIDWgtMap.clear();
}

////////////////////////////////////////////////////////////////////////

void PointWeightInfo::assign(const PointWeightInfo & m) {
   Type = m.Type;
   KDERefAngle = m.KDERefAngle;
   WriteWeights = m.WriteWeights;
   SIDWgtMap = m.SIDWgtMap;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void PointWeightInfo::add_sid(const string &sid, double lat, double lon) {

   // Add unique SID strings and initialize weights to bad data
   if(!SIDWgtMap.count(sid)) {
      SIDWgtMap[sid] = { lat, lon, bad_data_double };
   }
}

///////////////////////////////////////////////////////////////////////////////

bool PointWeightInfo::has_sid(const string &sid, double &wgt) const {
   bool found = false;

   if(SIDWgtMap.count(sid) == 0) {
      wgt = bad_data_double;
   }
   else {
      found = true;
      wgt = SIDWgtMap.at(sid).wgt;
   }

   return found;
}

///////////////////////////////////////////////////////////////////////////////

void PointWeightInfo::compute_weights() {

   // Compute KDE weights
   if(Type == PointWeightType::KDE) {


// JHG
   }

}

///////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
///////////////////////////////////////////////////////////////////////////////
 
PointWeightInfo parse_conf_point_weight(Dictionary *dict) {
   PointWeightInfo info;
   const char *method_name = "parse_conf_point_weight() -> ";
   
   if(!dict) {
      mlog << Error << "\n" << method_name
           << "empty dictionary!\n\n";
      exit(1);
   } 

   // Conf: point_weight_flag
   
   // Get the integer flag value for the current entry
   int v = dict->lookup_int(conf_key_point_weight_flag);

   // Parse config constant values
   MetConfig conf_const(replace_path(config_const_filename).c_str());

   // Convert integer to enumerated GridWeightType
        if(v == conf_const.lookup_int(conf_val_none)) info.set_type(PointWeightType::None);
   else if(v == conf_const.lookup_int(conf_val_sid))  info.set_type(PointWeightType::SID);
   else if(v == conf_const.lookup_int(conf_val_kde))  info.set_type(PointWeightType::KDE);
   else {
      mlog << Error << "\n" << method_name
           << "Unexpected config file value of " << v << " for \""
           << conf_key_point_weight_flag << "\".\n\n";
      exit(1);
   }  
   
   // Conf: kde_ref_angle
   info.set_kde_ref_angle(dict->lookup_double(conf_key_kde_ref_angle));

   // Conf: write_weights
   info.set_write_weights(dict->lookup_bool(conf_key_write_weights));

   return info;
}

///////////////////////////////////////////////////////////////////////////////
