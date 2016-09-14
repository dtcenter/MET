// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

#include "prob_ri_pair_info.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ProbRIPairInfo
//
////////////////////////////////////////////////////////////////////////

ProbRIPairInfo::ProbRIPairInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ProbRIPairInfo::~ProbRIPairInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ProbRIPairInfo::ProbRIPairInfo(const ProbRIPairInfo & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ProbRIPairInfo & ProbRIPairInfo::operator=(const ProbRIPairInfo & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ProbRIPairInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIPairInfo::clear() {

   ProbRI.clear();
   BDeck    = (TrackInfo *) 0;
   StormName.clear();
   BModel.clear();
   BLat     = BLon    = bad_data_double;
   ADLand   = BDLand  = bad_data_double;
   TrackErr = bad_data_double;
   XErr     = YErr    = bad_data_double;
   BBegV    = BEndV   = bad_data_double;
   BMinV    = BMaxV   = bad_data_double;
   BBegLev  = BEndLev = NoCycloneLevel;
   Line.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIPairInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "ProbRIPairInfo:\n"
       << prefix << "StormName = " << StormName << "\n"
       << prefix << "BModel    = " << BModel    << "\n"
       << prefix << "BLat      = " << BLat      << "\n"
       << prefix << "BLon      = " << BLon      << "\n"
       << prefix << "ADLand    = " << ADLand    << "\n"
       << prefix << "BDLand    = " << BDLand    << "\n"
       << prefix << "TrackErr  = " << TrackErr  << "\n"
       << prefix << "XErr      = " << XErr      << "\n"
       << prefix << "YErr      = " << YErr      << "\n"
       << prefix << "BBegV     = " << BBegV     << "\n"
       << prefix << "BBegLev   = " << BBegLev   << "\n"
       << prefix << "BEndV     = " << BEndV     << "\n"
       << prefix << "BEndLev   = " << BEndLev   << "\n"
       << prefix << "BMinV     = " << BMinV     << "\n"
       << prefix << "BMaxV     = " << BMaxV     << "\n"
       << prefix << "ProbRI: " << "\n";

   ProbRI.dump(out, indent_depth+1);

   out << prefix << "BDeck: " << "\n";
   if(BDeck) BDeck->dump(out, indent_depth+1);
   else      out << prefix << "(nul)";

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIPairInfo::case_info() const {
   ConcatString s;

   s << "ProbRIPairInfo: STORMID = " << ProbRI.storm_id()
     << ", ADECK = " << ProbRI.technique()
     << ", BDECK = " << BModel
     << ", INIT = " << unix_to_yyyymmdd_hhmmss(ProbRI.init())
     << ", RI_BEG = " << ProbRI.ri_beg()
     << ", RI_END = " << ProbRI.ri_end();

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIPairInfo::serialize() const {
   ConcatString s;

   s << "ProbRIPairInfo: "
     << "StormName = "  << StormName
     << ", BModel = "   << BModel
     << ", BLat = "     << BLat
     << ", BLon = "     << BLon
     << ", ADLand = "   << ADLand
     << ", BDLand = "   << BDLand
     << ", TrackErr = " << BBegV
     << ", XErr = "     << XErr
     << ", YErr = "     << YErr
     << ", BBegV = "    << BBegV
     << ", BBegLev = "  << cyclonelevel_to_string(BBegLev)
     << ", BEndV = "    << BEndV
     << ", BEndLev = "  << cyclonelevel_to_string(BEndLev)
     << ", BMinV = "    << BMinV
     << ", BMaxV = "    << BMaxV;

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIPairInfo::serialize_r(int n, int indent_depth) const {
   Indent prefix(indent_depth), prefix2(indent_depth+1);
   ConcatString s;
   int i;

   s << prefix << "[" << n << "] " << serialize() << "\n";

   s << prefix2 << "ProbRI = " << ProbRI.serialize() << "\n"
     << prefix2 << "BDeck  = " << (BDeck ? BDeck->serialize() : "(nul)")
     << "\n";

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbRIPairInfo::assign(const ProbRIPairInfo &p) {

   clear();

   ProbRI    = p.ProbRI;
   BDeck     = p.BDeck;
   StormName = p.StormName;
   BModel    = p.BModel;
   BLat      = p.BLat;
   BLon      = p.BLon;
   ADLand    = p.ADLand;
   BDLand    = p.BDLand;
   TrackErr  = p.TrackErr;
   XErr      = p.XErr;
   YErr      = p.YErr;
   BBegV     = p.BBegV;
   BEndV     = p.BEndV;
   BMinV     = p.BMinV;
   BMaxV     = p.BMaxV;
   BBegLev   = p.BBegLev;
   BEndLev   = p.BEndLev;
   Line      = p.Line;

   return;
}

////////////////////////////////////////////////////////////////////////

bool ProbRIPairInfo::set(const ProbRIInfo &prob_ri, const TrackInfo &bdeck) {
   int i, i_beg, i_end;

   clear();

   // Check for bad data
   if(prob_ri.init() == (unixtime) 0 ||
      is_bad_data(prob_ri.ri_beg())  ||
      is_bad_data(prob_ri.ri_end())) return(false);

   // Define begin and end times
   unixtime beg_ut = prob_ri.init() + (prob_ri.ri_beg() * sec_per_hour);
   unixtime end_ut = prob_ri.init() + (prob_ri.ri_end() * sec_per_hour);

   // Find matching BEST BEST track points
   for(i=0,i_beg=-1,i_end=-1; i<bdeck.n_points(); i++) {
      if(bdeck[i].valid() == beg_ut) i_beg = i;
      if(bdeck[i].valid() == end_ut) i_end = i;
   }

   // Check for matching points
   if(i_beg < 0 || i_end < 0) return(false);

   // Store the paired information
   ProbRI = prob_ri;
   BDeck  = &bdeck;

   StormName = bdeck.storm_name();
   BModel    = bdeck.technique();

   BLat   = bdeck[i_end].lat();
   BLon   = bdeck[i_end].lon();

   BBegV = bdeck[i_beg].v_max();
   BEndV = bdeck[i_end].v_max();

   // Compute the min and max wind speeds in the time window
   BMinV = BBegV;
   BMaxV = BBegV;
   for(i=i_beg; i<=i_end; i++) {
      if(bdeck[i].v_max() < BMinV) BMinV = bdeck[i].v_max();
      if(bdeck[i].v_max() > BMaxV) BMaxV = bdeck[i].v_max();
   }

   BBegLev = wind_speed_to_cyclonelevel(BBegV);
   BEndLev = wind_speed_to_cyclonelevel(BEndV);

   // Compute track errors
   latlon_to_xytk_err(prob_ri.lat(), prob_ri.lon(), BLat, BLon,
                      XErr, YErr, TrackErr);

   return(true);
}

////////////////////////////////////////////////////////////////////////

void ProbRIPairInfo::set(const TCStatLine &l) {
   double v;

   clear();

   // Check the line type
   if(l.type() != TCStatLineType_ProbRI) return;

   // Parse ProbRIInfo
   ProbRI.set(l);

   // Do not populate the BDECK
   BDeck = (TrackInfo *) 0;

   // Store column information
   StormName = l.get_item("STORM_NAME");
   BModel    = l.get_item("BMODEL");
   BLat      = atof(l.get_item("BLAT"));
   BLon      = atof(l.get_item("BLON"));
   TrackErr  = atof(l.get_item("TK_ERR"));
   XErr      = atof(l.get_item("X_ERR"));
   YErr      = atof(l.get_item("Y_ERR"));
   ADLand    = atof(l.get_item("ADLAND"));
   BDLand    = atof(l.get_item("BDLAND"));
   BBegV     = atof(l.get_item("BWIND_BEG"));
   BEndV     = atof(l.get_item("BWIND_END"));
   v         = atof(l.get_item("BDELTA_MAX"));
   BMinV     = (is_bad_data(BEndV) || is_bad_data(v) ?
               bad_data_double : BEndV - v);
   BMaxV     = BMinV;
   BBegLev   = string_to_cyclonelevel(l.get_item("BLEVEL_BEG"));
   BEndLev   = string_to_cyclonelevel(l.get_item("BLEVEL_END"));
   Line      = l;

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class ProbRIPairInfoArray
//
////////////////////////////////////////////////////////////////////////

ProbRIPairInfoArray::ProbRIPairInfoArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ProbRIPairInfoArray::~ProbRIPairInfoArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ProbRIPairInfoArray::ProbRIPairInfoArray(const ProbRIPairInfoArray & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ProbRIPairInfoArray & ProbRIPairInfoArray::operator=(const ProbRIPairInfoArray & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ProbRIPairInfoArray::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIPairInfoArray::clear() {

   // Erase the entire vector
   Pairs.erase(Pairs.begin(), Pairs.end());

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIPairInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "ProbRIPairInfoArray:\n"
       << prefix << "NPairs = " << n_pairs() << "\n";

   for(i=0; i<Pairs.size(); i++) {
      out << prefix << "Pair[" << i+1 << "]:\n";
      Pairs[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIPairInfoArray::serialize() const {
   ConcatString s;

   s << "ProbRIPairInfoArray: "
     << "NPairs = " << n_pairs();

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIPairInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;

   s << prefix << serialize() << ", Pairs:\n";

   for(int i=0; i<Pairs.size(); i++) {
      s << Pairs[i].serialize_r(i+1, indent_depth+1);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbRIPairInfoArray::assign(const ProbRIPairInfoArray &p) {

   clear();

   // Allocate space and copy each element
   for(int i=0; i<p.Pairs.size(); i++) {
      add(p.Pairs[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

const ProbRIPairInfo & ProbRIPairInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= Pairs.size())) {
      mlog << Error
           << "\nProbRIPairInfoArray::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Pairs[n]);
}

////////////////////////////////////////////////////////////////////////

void ProbRIPairInfoArray::add(const ProbRIPairInfo &p) {

   Pairs.push_back(p);

   return;
}

////////////////////////////////////////////////////////////////////////

bool ProbRIPairInfoArray::add(const ProbRIInfo &p, const TrackInfo &t) {
   ProbRIPairInfo pair;

   // Attempt to set a new pair
   if(!pair.set(p, t)) return(false);

   Pairs.push_back(pair);

   return(true);
}

////////////////////////////////////////////////////////////////////////

