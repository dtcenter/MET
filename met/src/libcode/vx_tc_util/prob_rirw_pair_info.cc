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

#include "prob_rirw_pair_info.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ProbRIRWPairInfo
//
////////////////////////////////////////////////////////////////////////

ProbRIRWPairInfo::ProbRIRWPairInfo() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ProbRIRWPairInfo::~ProbRIRWPairInfo() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ProbRIRWPairInfo::ProbRIRWPairInfo(const ProbRIRWPairInfo & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ProbRIRWPairInfo & ProbRIRWPairInfo::operator=(const ProbRIRWPairInfo & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWPairInfo::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWPairInfo::clear() {

   ProbRIRW.clear();
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

void ProbRIRWPairInfo::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   out << prefix << "ProbRIRWPairInfo:\n"
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
       << prefix << "ProbRIRW: " << "\n";

   ProbRIRW.dump(out, indent_depth+1);

   out << prefix << "BDeck: " << "\n";
   if(BDeck) BDeck->dump(out, indent_depth+1);
   else      out << prefix << "(nul)";

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIRWPairInfo::case_info() const {
   ConcatString s;

   s << "ProbRIRWPairInfo: STORMID = " << ProbRIRW.storm_id()
     << ", ADECK = " << ProbRIRW.technique()
     << ", BDECK = " << BModel
     << ", INIT = " << unix_to_yyyymmdd_hhmmss(ProbRIRW.init())
     << ", RIRW_BEG = " << ProbRIRW.rirw_beg()
     << ", RIRW_END = " << ProbRIRW.rirw_end();

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIRWPairInfo::serialize() const {
   ConcatString s;

   s << "ProbRIRWPairInfo: "
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

ConcatString ProbRIRWPairInfo::serialize_r(int n, int indent_depth) const {
   Indent prefix(indent_depth), prefix2(indent_depth+1);
   ConcatString s;

   s << prefix << "[" << n << "] " << serialize() << "\n";

   s << prefix2 << "ProbRIRW = " << ProbRIRW.serialize() << "\n"
     << prefix2 << "BDeck  = " << (BDeck ? BDeck->serialize().text() : "(nul)")
     << "\n";

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWPairInfo::assign(const ProbRIRWPairInfo &p) {

   clear();

   ProbRIRW  = p.ProbRIRW;
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

bool ProbRIRWPairInfo::set(const ProbRIRWInfo &prob_rirw_info,
                           const TrackInfo &bdeck_info) {
   int i, i_beg, i_end;

   clear();

   // Check for bad data
   if(prob_rirw_info.init() == (unixtime) 0  ||
      is_bad_data(prob_rirw_info.rirw_beg()) ||
      is_bad_data(prob_rirw_info.rirw_end())) return(false);

   // Define begin and end times
   unixtime beg_ut = prob_rirw_info.init() + (prob_rirw_info.rirw_beg() * sec_per_hour);
   unixtime end_ut = prob_rirw_info.init() + (prob_rirw_info.rirw_end() * sec_per_hour);

   // Find matching BEST BEST track points
   for(i=0,i_beg=-1,i_end=-1; i<bdeck_info.n_points(); i++) {
      if(bdeck_info[i].valid() == beg_ut) i_beg = i;
      if(bdeck_info[i].valid() == end_ut) i_end = i;
   }

   // Check for matching points
   if(i_beg < 0 || i_end < 0) return(false);

   // Store the paired information
   ProbRIRW = prob_rirw_info;
   BDeck    = &bdeck_info;

   StormName = bdeck_info.storm_name();
   BModel    = bdeck_info.technique();

   BLat  = bdeck_info[i_end].lat();
   BLon  = bdeck_info[i_end].lon();

   BBegV = bdeck_info[i_beg].v_max();
   BEndV = bdeck_info[i_end].v_max();

   // Compute the min and max wind speeds in the time window
   BMinV = BBegV;
   BMaxV = BBegV;
   for(i=i_beg; i<=i_end; i++) {
      if(bdeck_info[i].v_max() < BMinV) BMinV = bdeck_info[i].v_max();
      if(bdeck_info[i].v_max() > BMaxV) BMaxV = bdeck_info[i].v_max();
   }

   BBegLev = wind_speed_to_cyclonelevel(BBegV);
   BEndLev = wind_speed_to_cyclonelevel(BEndV);

   // Compute track errors
   latlon_to_xytk_err(prob_rirw_info.lat(), prob_rirw_info.lon(), BLat, BLon,
                      XErr, YErr, TrackErr);

   return(true);
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWPairInfo::set(const TCStatLine &l) {
   double v;

   clear();

   // Check the line type
   if(l.type() != TCStatLineType_ProbRIRW) return;

   // Parse ProbRIRWInfo
   ProbRIRW.set(l);

   // Do not populate the BDECK
   BDeck = (TrackInfo *) 0;

   // Store column information
   StormName = l.get_item("STORM_NAME", false);
   BModel    = l.get_item("BMODEL", false);
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
//  Code for class ProbRIRWPairInfoArray
//
////////////////////////////////////////////////////////////////////////

ProbRIRWPairInfoArray::ProbRIRWPairInfoArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ProbRIRWPairInfoArray::~ProbRIRWPairInfoArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ProbRIRWPairInfoArray::ProbRIRWPairInfoArray(const ProbRIRWPairInfoArray & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ProbRIRWPairInfoArray & ProbRIRWPairInfoArray::operator=(const ProbRIRWPairInfoArray & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWPairInfoArray::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWPairInfoArray::clear() {

   // Erase the entire vector
   Pairs.erase(Pairs.begin(), Pairs.end());

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWPairInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);

   out << prefix << "ProbRIRWPairInfoArray:\n"
       << prefix << "NPairs = " << n_pairs() << "\n";

   for(unsigned int i=0; i<Pairs.size(); i++) {
      out << prefix << "Pair[" << i+1 << "]:\n";
      Pairs[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIRWPairInfoArray::serialize() const {
   ConcatString s;

   s << "ProbRIRWPairInfoArray: "
     << "NPairs = " << n_pairs();

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbRIRWPairInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;

   s << prefix << serialize() << ", Pairs:\n";

   for(unsigned int i=0; i<Pairs.size(); i++) {
      s << Pairs[i].serialize_r(i+1, indent_depth+1);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWPairInfoArray::assign(const ProbRIRWPairInfoArray &p) {

   clear();

   // Allocate space and copy each element
   for(unsigned int i=0; i<p.Pairs.size(); i++) {
      add(p.Pairs[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

const ProbRIRWPairInfo & ProbRIRWPairInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= (int) Pairs.size())) {
      mlog << Error
           << "\nProbRIRWPairInfoArray::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Pairs[n]);
}

////////////////////////////////////////////////////////////////////////

void ProbRIRWPairInfoArray::add(const ProbRIRWPairInfo &p) {

   Pairs.push_back(p);

   return;
}

////////////////////////////////////////////////////////////////////////

bool ProbRIRWPairInfoArray::add(const ProbRIRWInfo &p, const TrackInfo &t) {
   ProbRIRWPairInfo pair;

   // Attempt to set a new pair
   if(!pair.set(p, t)) return(false);

   Pairs.push_back(pair);

   return(true);
}

////////////////////////////////////////////////////////////////////////

