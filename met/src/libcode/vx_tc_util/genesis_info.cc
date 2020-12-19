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
#include <string.h>
#include <cstdio>
#include <cmath>

#include "nav.h"

#include "genesis_info.h"
#include "vx_config.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for struct GenesisEventInfo
//
////////////////////////////////////////////////////////////////////////

void GenesisEventInfo::clear() {
   Technique.clear();
   Category.clear();
   VMaxThresh.clear();
   MSLPThresh.clear();
}

////////////////////////////////////////////////////////////////////////

bool GenesisEventInfo::is_genesis(const TrackPoint &p) const {
   bool status = true;

   // Check event category
   if(Category.size() > 0 &&
      std::count(Category.begin(), Category.end(), p.level()) == 0) {
      status = false;
   }

   // Check VMax threshold
   if(VMaxThresh.get_type() != thresh_na &&
      !VMaxThresh.check(p.v_max())) {
      status = false;
   }

   // Check MSLP threshold
   if(MSLPThresh.get_type() != thresh_na &&
      !MSLPThresh.check(p.mslp())) {
      status = false;
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

GenesisEventInfo parse_conf_genesis_event_info(Dictionary *dict) {
   GenesisEventInfo info;
   StringArray sa;
   int i;

   if(!dict) {
      mlog << Error << "\nparse_conf_genesis_event_info() -> "
           << "empty dictionary!\n\n";
      exit(1);
   }

   // Conf: technique (optional)
   info.Technique = dict->lookup_string(conf_key_technique, false);

   // Conf: category (optional)
   sa = dict->lookup_string_array(conf_key_category, false);
   for(i=0; i<sa.n(); i++) {
      info.Category.push_back(string_to_cyclonelevel(sa[i].c_str()));
   }

   // Conf: vmax_thresh
   info.VMaxThresh = dict->lookup_thresh(conf_key_vmax_thresh);

   // Conf: mslp_thresh
   info.MSLPThresh = dict->lookup_thresh(conf_key_mslp_thresh);

   return(info);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class GenesisInfo
//
////////////////////////////////////////////////////////////////////////

GenesisInfo::GenesisInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

GenesisInfo::~GenesisInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

GenesisInfo::GenesisInfo(const GenesisInfo &g) {

   clear();

   assign(g);
}

////////////////////////////////////////////////////////////////////////

GenesisInfo & GenesisInfo::operator=(const GenesisInfo &g) {

   if(this == &g) return(*this);

   assign(g);

   return(*this);
}

////////////////////////////////////////////////////////////////////////
//
// Check for matching stormid, technique name/number, timing
// information, and location information.
//
////////////////////////////////////////////////////////////////////////

bool GenesisInfo::operator==(const GenesisInfo & g) const {

   return(StormId         == g.StormId         &&
          Technique       == g.Technique       &&
          TechniqueNumber == g.TechniqueNumber &&
          GenesisTime     == g.GenesisTime     &&
          InitTime        == g.InitTime        &&
          LeadTime        == g.LeadTime        &&
          is_eq(Lat, g.Lat)                    &&
          is_eq(Lon, g.Lon));
}

////////////////////////////////////////////////////////////////////////
//
// Check for matching technique name/number, timing information, and
// location information. Do not check the basin and storm number.
//
////////////////////////////////////////////////////////////////////////

bool GenesisInfo::is_storm(const GenesisInfo & g) const {

   return(Technique       == g.Technique       &&
          TechniqueNumber == g.TechniqueNumber &&
          GenesisTime     == g.GenesisTime     &&
          InitTime        == g.InitTime        &&
          LeadTime        == g.LeadTime        &&
          is_eq(Lat, g.Lat)                    &&
          is_eq(Lon, g.Lon));
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::clear() {

   IsSet       = false;
   IsBestTrack = false;
   IsOperTrack = false;
   IsAnlyTrack = false;

   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();
   TechniqueNumber = bad_data_int;
   Technique.clear();
   Initials.clear();

   GenesisTime     = (unixtime) 0;
   InitTime        = (unixtime) 0;
   LeadTime        = bad_data_int;

   Lat             = bad_data_double;
   Lon             = bad_data_double;
   DLand           = bad_data_double;

   NPoints         = 0;
   MinValidTime    = (unixtime) 0;
   MaxValidTime    = (unixtime) 0;
   MinWarmCoreTime = (unixtime) 0;
   MaxWarmCoreTime = (unixtime) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::assign(const GenesisInfo &g) {

   clear();

   IsSet           = true;
   IsBestTrack     = g.IsBestTrack;
   IsOperTrack     = g.IsOperTrack;
   IsAnlyTrack     = g.IsAnlyTrack;

   StormId         = g.StormId;
   Basin           = g.Basin;
   Cyclone         = g.Cyclone;
   StormName       = g.StormName;
   TechniqueNumber = g.TechniqueNumber;
   Technique       = g.Technique;
   Initials        = g.Initials;

   GenesisTime     = g.GenesisTime;
   InitTime        = g.InitTime;
   LeadTime        = g.LeadTime;

   Lat             = g.Lat;
   Lon             = g.Lon;
   DLand           = g.DLand;

   NPoints         = g.NPoints;
   MinValidTime    = g.MinValidTime;
   MaxValidTime    = g.MaxValidTime;
   MinWarmCoreTime = g.MinWarmCoreTime;
   MaxWarmCoreTime = g.MaxWarmCoreTime;

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   out << prefix << "IsSet           = " << bool_to_string(IsSet) << "\n";
   out << prefix << "IsBest          = " << bool_to_string(IsBestTrack) << "\n";
   out << prefix << "IsOper          = " << bool_to_string(IsOperTrack) << "\n";
   out << prefix << "IsAnly          = " << bool_to_string(IsAnlyTrack) << "\n";
   out << prefix << "StormId         = \"" << StormId.contents() << "\"\n";
   out << prefix << "Basin           = \"" << Basin.contents() << "\"\n";
   out << prefix << "Cyclone         = \"" << Cyclone.contents() << "\"\n";
   out << prefix << "StormName       = \"" << StormName.contents() << "\"\n";
   out << prefix << "TechniqueNumber = " << TechniqueNumber << "\n";
   out << prefix << "Technique       = \"" << Technique.contents() << "\"\n";
   out << prefix << "Initials        = \"" << Initials.contents() << "\"\n";
   out << prefix << "GenesisTime         = \""
                 << (GenesisTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(GenesisTime).text() :
                     na_str) << "\"\n";
   out << prefix << "InitTime        = \""
                 << (InitTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(InitTime).text() :
                     na_str) << "\"\n";
   out << prefix << "LeadTime        = \""
                 << sec_to_hhmmss(LeadTime).text() << "\"\n";
   out << prefix << "Lat             = " << Lat << "\n";
   out << prefix << "Lon             = " << Lon << "\n";
   out << prefix << "DLand           = " << DLand << "\n";
   out << prefix << "NPoints         = " << NPoints << "\n";
   out << prefix << "MinValidTime    = \""
                 << (MinValidTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(MinValidTime).text() :
                     na_str) << "\"\n";
   out << prefix << "MaxValidTime    = \""
                 << (MaxValidTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(MaxValidTime).text() :
                     na_str) << "\"\n";
   out << prefix << "MinWarmCoreTime = \""
                 << (MinWarmCoreTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(MinWarmCoreTime).text() :
                     na_str) << "\"\n";
   out << prefix << "MaxWarmCoreTime = \""
                 << (MaxWarmCoreTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(MaxWarmCoreTime).text() :
                     na_str) << "\"\n";

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfo::serialize() const {
   ConcatString s;

   s << "GenesisInfo: "
     << "IsSet = " << bool_to_string(IsSet)
     << ", IsBest = " << bool_to_string(IsBestTrack)
     << ", IsOper = " << bool_to_string(IsOperTrack)
     << ", IsAnly = " << bool_to_string(IsAnlyTrack)
     << ", StormId = \"" << StormId.contents() << "\""
     << ", Basin = \"" << Basin.contents() << "\""
     << ", Cyclone = \"" << Cyclone.contents() << "\""
     << ", StormName = \"" << StormName.contents() << "\""
     << ", TechniqueNumber = " << TechniqueNumber
     << ", Technique = \"" << Technique.contents() << "\""
     << ", Initials = \"" << Initials.contents() << "\""
     << ", GenesisTime = \"" << (GenesisTime > 0 ?
           unix_to_yyyymmdd_hhmmss(GenesisTime).text() : na_str) << "\""
     << ", InitTime = \"" << (InitTime > 0 ?
           unix_to_yyyymmdd_hhmmss(InitTime).text() : na_str) << "\""
     << ", LeadTime = \"" << sec_to_hhmmss(LeadTime).text() << "\""
     << ", Lat = " << Lat
     << ", Lon = " << Lon
     << ", DLand = " << DLand
     << ", NPoints = " << NPoints
     << ", MinValidTime = \"" << (MinValidTime > 0 ?
           unix_to_yyyymmdd_hhmmss(MinValidTime).text() : na_str) << "\""
     << ", MaxValidTime = \"" << (MaxValidTime > 0 ?
           unix_to_yyyymmdd_hhmmss(MaxValidTime).text() : na_str) << "\""
     << ", MinWarmCoreTime = \"" << (MinWarmCoreTime > 0 ?
           unix_to_yyyymmdd_hhmmss(MinWarmCoreTime).text() : na_str) << "\""
     << ", MaxWarmCoreTime = \"" << (MaxWarmCoreTime > 0 ?
           unix_to_yyyymmdd_hhmmss(MaxWarmCoreTime).text() : na_str) << "\"";

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfo::serialize_r(int n, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;

   s << prefix << "[" << n << "] " << serialize();

   return(s);

}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::set_storm_id() {

   StormId = define_storm_id(InitTime, MinValidTime, MaxValidTime,
                             Basin, Cyclone);

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::set_dland(double d) {
   DLand = d;
   return;
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfo::set(const TrackInfo &t,
                      const GenesisEventInfo *event_info) {
   int i;
   int i_point = bad_data_int;

   // Initialize
   clear();

   // Check for NULL pointer
   if(event_info == 0) return(false);

   // Find genesis point
   for(i=0; i<t.n_points(); i++) {
      if(event_info->is_genesis(t[i])) {
         i_point = i;
         break;
      }
   }

   // Return bad status if genesis was not found
   if(is_bad_data(i_point)) return(false);

   // Store track type information
   IsBestTrack = t.is_best_track();
   IsOperTrack = t.is_oper_track();
   IsAnlyTrack = t.is_anly_track();

   // Initialize
   IsSet           = true;
   Basin           = t.basin();
   Cyclone         = t.cyclone();
   StormName       = t.storm_name();
   TechniqueNumber = t.technique_number();
   Technique       = t.technique();
   Initials        = t.initials();

   // Store genesis time and location.
   GenesisTime = t[i_point].valid();
   Lat         = t[i_point].lat();
   Lon         = t[i_point].lon();

   // For analysis tracks, keep InitTime = LeadTime = 0.
   if(IsAnlyTrack) {
      InitTime = (unixtime) 0;
      LeadTime = 0;
   }
   else {
      InitTime = t.init();
      LeadTime = t[i_point].lead();
   }

   // Compute the track time ranges
   MinValidTime    = MaxValidTime    = t[0].valid();
   MinWarmCoreTime = MaxWarmCoreTime = 0;
   NPoints         = t.n_points();

   for(i=0; i<NPoints; i++) {

      if(t[i].valid() < MinValidTime) MinValidTime = t[i].valid();
      if(t[i].valid() > MaxValidTime) MaxValidTime = t[i].valid();

      if(t[i].warm_core()) {
         if(MinWarmCoreTime == 0 || t[i].valid() < MinWarmCoreTime) {
            MinWarmCoreTime = t[i].valid();
         }
         if(MaxWarmCoreTime == 0 || t[i].valid() > MaxWarmCoreTime) {
            MaxWarmCoreTime = t[i].valid();
         }
      }
   }

   // Create the storm id
   set_storm_id();

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfo::is_match(const TrackPoint &p,
                           const double rad) const {

   // Check for matching in time and space
   return(GenesisTime == p.valid() &&
          gc_dist(Lat, Lon, p.lat(), p.lon()) <= rad);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class GenesisInfoArray
//
////////////////////////////////////////////////////////////////////////

GenesisInfoArray::GenesisInfoArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GenesisInfoArray::~GenesisInfoArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

GenesisInfoArray::GenesisInfoArray(const GenesisInfoArray & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

GenesisInfoArray & GenesisInfoArray::operator=(const GenesisInfoArray & t) {

   if(this == &t)  return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void GenesisInfoArray::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfoArray::clear() {

   Genesis.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "NGenesis = " << (int) Genesis.size() << "\n";

   for(i=0; i<Genesis.size(); i++) {
      out << prefix << "GenesisInfo[" << i+1 << "]:" << "\n";
      Genesis[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfoArray::serialize() const {
   ConcatString s;

   s << "GenesisInfoArray: "
     << "NGenesis = " << (int) Genesis.size();

   return(s);

}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   s << prefix << serialize() << ", Genesis:\n";

   for(i=0; i<Genesis.size(); i++) {
      s << Genesis[i].serialize_r(i+1, indent_depth+1) << "\n";
   }

   return(s);

}

////////////////////////////////////////////////////////////////////////

void GenesisInfoArray::assign(const GenesisInfoArray &ga) {
   int i;

   clear();

   if(ga.Genesis.size() == 0) return;

   for(i=0; i<ga.Genesis.size(); i++) add(ga[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

const GenesisInfo & GenesisInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= Genesis.size())) {
      mlog << Error
           << "\nGenesisInfoArray::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Genesis[n]);
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfoArray::add(const GenesisInfo &gi) {

   // Ignore true duplicates
   if(has(gi)) {
      mlog << Warning << "\nGenesisInfoArray::add() -> "
           << "Skipping duplicate genesis event:\n" << gi.serialize()
           << "\n\n";
      return(false);
   }

   // Print warning for matches
   if(has_storm(gi)) {
      mlog << Warning << "\nGenesisInfoArray::add() -> "
           << "Including multiple genesis events for the same storm:\n"
           << gi.serialize() << "\n\n";
   }

   // Store the genesis object
   Genesis.push_back(gi);

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfoArray::has(const GenesisInfo &g) {

   for(int i=0; i<Genesis.size(); i++) {
      if(g == Genesis[i]) return(true);
   }

   return(false);
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfoArray::has_storm(const GenesisInfo &g) {

   for(int i=0; i<Genesis.size(); i++) {
      if(g.is_storm(Genesis[i])) return(true);
   }

   return(false);
}

////////////////////////////////////////////////////////////////////////

void GenesisInfoArray::set_dland(int n, double d) {

   // Check range
   if((n < 0) || (n >= Genesis.size())) {
      mlog << Error
           << "\nGenesisInfoArray::set_dland() -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   Genesis[n].set_dland(d);

   return;
}

////////////////////////////////////////////////////////////////////////

int GenesisInfoArray::n_technique() const {
   StringArray sa;
   int i, n;

   for(i=0, n=0; i<Genesis.size(); i++) {
      if(!sa.has(Genesis[i].technique())) {
         sa.add(Genesis[i].technique());
         n++;
      }
   }

   return(n);
}

////////////////////////////////////////////////////////////////////////
