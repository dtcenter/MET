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

   if(!Track || !g.Track) return(false);

   return(Track->storm_id()         == g.Track->storm_id()         &&
          Technique                 == g.Technique                 &&
          Track->technique_number() == g.Track->technique_number() &&
          GenesisTime               == g.GenesisTime               &&
          InitTime                  == g.InitTime                  &&
          LeadTime                  == g.LeadTime                  &&
          is_eq(Lat, g.Lat)                                        &&
          is_eq(Lon, g.Lon));
}

////////////////////////////////////////////////////////////////////////
//
// Check for matching technique name/number, timing information, and
// location information. Do not check the basin and storm number.
//
////////////////////////////////////////////////////////////////////////

bool GenesisInfo::is_storm(const GenesisInfo & g) const {

   if(!Track || !g.Track) return(false);

   return(Technique                 == g.Technique                 &&
          Track->technique_number() == g.Track->technique_number() &&
          GenesisTime               == g.GenesisTime               &&
          InitTime                  == g.InitTime                  &&
          LeadTime                  == g.LeadTime                  &&
          is_eq(Lat, g.Lat)                                        &&
          is_eq(Lon, g.Lon));
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::clear() {

   IsSet           = false;

   Track           = (TrackInfo *) 0;
   GenIndex        = bad_data_int;

   Technique.clear();

   GenesisTime     = (unixtime) 0;
   InitTime        = (unixtime) 0;
   LeadTime        = bad_data_int;

   Lat             = bad_data_double;
   Lon             = bad_data_double;
   DLand           = bad_data_double;

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::assign(const GenesisInfo &g) {

   clear();

   IsSet           = true;

   Track           = g.Track;
   GenIndex        = g.GenIndex;

   Technique       = g.Technique;
   GenesisTime     = g.GenesisTime;
   InitTime        = g.InitTime;
   LeadTime        = g.LeadTime;

   Lat             = g.Lat;
   Lon             = g.Lon;
   DLand           = g.DLand;

   return;
}

////////////////////////////////////////////////////////////////////////

void GenesisInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   out << prefix << "IsSet       = " << bool_to_string(IsSet) << "\n";
   out << prefix << "StormId     = \""<< storm_id() << "\"\n";
   out << prefix << "Technique   = \""<< Technique << "\"\n";
   out << prefix << "GenesisTime = \""
                 << (GenesisTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(GenesisTime).text() :
                     na_str) << "\"\n";
   out << prefix << "InitTime    = \""
                 << (InitTime > 0 ?
                     unix_to_yyyymmdd_hhmmss(InitTime).text() :
                     na_str) << "\"\n";
   out << prefix << "LeadTime    = \""
                 << sec_to_hhmmss(LeadTime).text() << "\"\n";
   out << prefix << "Lat         = " << Lat << "\n";
   out << prefix << "Lon         = " << Lon << "\n";
   out << prefix << "DLand       = " << DLand << "\n";

   out << flush;

   return;

}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfo::serialize() const {
   ConcatString s;

   s << "GenesisInfo: "
     << "IsSet = " << bool_to_string(IsSet)
     << ", StormId = \"" << storm_id() << "\""
     << ", Technique = \"" << Technique << "\""
     << ", GenesisTime = \"" << (GenesisTime > 0 ?
           unix_to_yyyymmdd_hhmmss(GenesisTime).text() : na_str) << "\""
     << ", InitTime = \"" << (InitTime > 0 ?
           unix_to_yyyymmdd_hhmmss(InitTime).text() : na_str) << "\""
     << ", LeadTime = \"" << sec_to_hhmmss(LeadTime).text() << "\""
     << ", Lat = " << Lat
     << ", Lon = " << Lon
     << ", DLand = " << DLand;

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

void GenesisInfo::set_dland(double d) {
   DLand = d;
   return;
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfo::set(const TrackInfo *t,
                      const GenesisEventInfo *event_info) {
   int i;

   // Initialize
   clear();

   // Check for NULL pointer
   if(!t || !event_info) return(false);

   // Find genesis point
   for(i=0, GenIndex=bad_data_int; i<t->n_points(); i++) {
      if(event_info->is_genesis((*t)[i])) {
         GenIndex = i;
         break;
      }
   }

   // Return bad status if genesis was not found
   if(is_bad_data(GenIndex)) return(false);

   // Initialize
   IsSet = true;

   // Store track pointer
   Track = t;

   // Store genesis time and location.
   Technique   = t->technique();
   GenesisTime = (*t)[GenIndex].valid();
   Lat         = (*t)[GenIndex].lat();
   Lon         = (*t)[GenIndex].lon();

   // For analysis tracks, keep InitTime = LeadTime = 0.
   if(Track->is_anly_track()) {
      InitTime = (unixtime) 0;
      LeadTime = 0;
   }
   else {
      InitTime = t->init();
      LeadTime = (*t)[GenIndex].lead();
   }

   return(true);
}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfo::storm_id() const {
   return(Track ? Track->storm_id() : na_str);
}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfo::basin() const {
   return(Track ? Track->basin() : na_str);
}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfo::cyclone() const {
   return(Track ? Track->cyclone() : na_str);
}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfo::storm_name() const {
   return(Track ? Track->storm_name() : na_str);
}

////////////////////////////////////////////////////////////////////////

ConcatString GenesisInfo::technique() const {
   return(Technique);
}

////////////////////////////////////////////////////////////////////////

unixtime GenesisInfo::valid_min() const {
   return(Track ? Track->valid_min() : (unixtime) 0);
}

////////////////////////////////////////////////////////////////////////

unixtime GenesisInfo::valid_max() const {
   return(Track ? Track->valid_max() : (unixtime) 0);
}

////////////////////////////////////////////////////////////////////////

int GenesisInfo::duration() const {
   return(Track ? Track->duration() : bad_data_int);
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
   int i;

   // Skip true duplicates
   if(has(gi)) {
      mlog << Warning << "\nGenesisInfoArray::add() -> "
           << "Skipping duplicate genesis event:\n"
           << gi.serialize() << "\n\n";
      return(false);
   }

   // Store the genesis object
   Genesis.push_back(gi);

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfoArray::has(const GenesisInfo &g) const {

   for(int i=0; i<Genesis.size(); i++) {
      if(g == Genesis[i]) return(true);
   }

   return(false);
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfoArray::has_storm(const GenesisInfo &g, int &i) const {

   for(i=0; i<Genesis.size(); i++) {
      if(g.is_storm(Genesis[i])) return(true);
   }

   i = bad_data_int;
   return(false);
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfoArray::has_storm_id(const ConcatString &s, int &i) const {

   for(i=0; i<Genesis.size(); i++) {
      if(Genesis[i].track()->storm_id() == s) return(true);
   }

   i = bad_data_int;
   return(false);
}

////////////////////////////////////////////////////////////////////////

bool GenesisInfoArray::erase_storm_id(const ConcatString &s) {
   int i;
   bool status;

   if((status = has_storm_id(s, i))) {
      Genesis.erase(Genesis.begin()+i);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

int GenesisInfoArray::n_technique() const {
   StringArray sa;

   for(int i=0; i<Genesis.size(); i++) {
      if(!sa.has(Genesis[i].technique())) {
         sa.add(Genesis[i].technique());
      }
   }

   return(sa.n());
}

////////////////////////////////////////////////////////////////////////
