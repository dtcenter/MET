// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#include "vx_util.h"

#include "mask_sid.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
// Code for MaskSID struct
//
///////////////////////////////////////////////////////////////////////////////

MaskSID::MaskSID() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

MaskSID::~MaskSID() {
   clear();
}

////////////////////////////////////////////////////////////////////////

MaskSID::MaskSID(const MaskSID &m) {

   init_from_scratch();

   assign(m);
}

////////////////////////////////////////////////////////////////////////

MaskSID & MaskSID::operator=(const MaskSID &m) noexcept {

   if(this == &m) return *this;

   assign(m);

   return *this;
}

////////////////////////////////////////////////////////////////////////

void MaskSID::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void MaskSID::clear() {
   Name.clear();
   HasWeights = false;
   SIDMap.clear();
}

////////////////////////////////////////////////////////////////////////

void MaskSID::assign(const MaskSID & m) {
   Name = m.Name;
   HasWeights = m.HasWeights;
   SIDMap = m.SIDMap;

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool MaskSID::operator==(const MaskSID &m) const {
   bool match = true;

   if(!(Name   == m.Name   ) ||
      !(SIDMap == m.SIDMap)) {
      match = false;
   }

   return match;
}

///////////////////////////////////////////////////////////////////////////////

const std::map<std::string,double> & MaskSID::sid_map() const {
   return SIDMap;
}

///////////////////////////////////////////////////////////////////////////////

void MaskSID::set_name(const string &s) {
   Name = s;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void MaskSID::add(const string &text) {
   ConcatString sid(text);

   // Default weight value of 1.0
   double weight = 1.0;

   // Check for optional weight
   StringArray sa(sid.split("("));
   if(sa.n() > 1) {
      sid = sa[0];
      weight = stod(sa[1]);
      HasWeights = true;
   }

   // Add station ID map entry
   if(SIDMap.count(sid) == 0) SIDMap[sid] = weight;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void MaskSID::add_css(const string &text) {
   StringArray sa;
   sa.add_css(text);
   for(int i=0; i<sa.n(); i++) add(sa[i]);
   return;
}

///////////////////////////////////////////////////////////////////////////////

bool MaskSID::has_sid(const string &s) const {
   return SIDMap.count(s) > 0;
}

///////////////////////////////////////////////////////////////////////////////

bool MaskSID::has_sid(const string &s, double &weight) const {
   bool found = false;

   if(SIDMap.count(s) == 0) {
      weight = bad_data_double;
   }
   else {
      found = true;
      weight = SIDMap.at(s);
   }

   return found;
}

///////////////////////////////////////////////////////////////////////////////

