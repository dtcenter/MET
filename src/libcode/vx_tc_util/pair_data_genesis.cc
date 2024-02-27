// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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

#include "pair_data_genesis.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for enum GenesisPairCategory
//
////////////////////////////////////////////////////////////////////////

ConcatString genesispaircategory_to_string(const GenesisPairCategory c) {
   const char *s = (const char *) nullptr;

   switch(c) {
      case FYOYGenesis:    s = "FYOY";    break;
      case FYONGenesis:    s = "FYON";    break;
      case FNOYGenesis:    s = "FNOY";    break;
      case DiscardGenesis: s = "DISCARD"; break;
      default:             s = na_str;    break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////
//
//  Code for struct Genesis Pair Differences
//
////////////////////////////////////////////////////////////////////////

GenesisPairDiff::GenesisPairDiff() {

   clear();
}

////////////////////////////////////////////////////////////////////////

void GenesisPairDiff::clear() {

   DevDist     = bad_data_double;
   DevDSec     = bad_data_int;
   OpsDSec     = bad_data_int;
   DevCategory = NoGenesisPairCategory;
   OpsCategory = NoGenesisPairCategory;

   return;
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class PairDataGenesis
//
////////////////////////////////////////////////////////////////////////

PairDataGenesis::PairDataGenesis() {

   clear();
}

////////////////////////////////////////////////////////////////////////

PairDataGenesis::~PairDataGenesis() {

   clear();
}

////////////////////////////////////////////////////////////////////////

PairDataGenesis::PairDataGenesis(const PairDataGenesis &g) {

   clear();

   assign(g);
}

////////////////////////////////////////////////////////////////////////

PairDataGenesis & PairDataGenesis::operator=(const PairDataGenesis &g) {

   if(this == &g) return(*this);

   assign(g);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void PairDataGenesis::clear() {

   Desc.clear();
   Mask.clear();
   Model.clear();

   NPair = 0;

   BestStormId.clear();
   InitTime.clear();
   LeadTime.clear();

   FcstGen.clear();
   BestGen.clear();
   GenDiff.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataGenesis::assign(const PairDataGenesis &g) {

   clear();

   Desc        = g.Desc;
   Mask        = g.Mask;
   Model       = g.Model;

   NPair       = g.NPair;

   BestStormId = g.BestStormId;
   InitTime    = g.InitTime;
   LeadTime    = g.LeadTime;

   FcstGen     = g.FcstGen;
   BestGen     = g.BestGen;
   GenDiff     = g.GenDiff;

   return;
}

////////////////////////////////////////////////////////////////////////

const GenesisInfo * PairDataGenesis::fcst_gen(int i) const {

   if(i < 0 || i > NPair) {
      mlog << Error << "\nPairBase::fcst_gen() -> "
           << "range check error: " << i << " not in (0, "
           << NPair << ").\n\n";
      exit(1);
   }

   return(FcstGen[i]);
}

////////////////////////////////////////////////////////////////////////

const GenesisInfo * PairDataGenesis::best_gen(int i) const {

   if(i < 0 || i > NPair) {
      mlog << Error << "\nPairBase::best_gen() -> "
           << "range check error: " << i << " not in (0, "
           << NPair << ").\n\n";
      exit(1);
   }

   return(BestGen[i]);
}

////////////////////////////////////////////////////////////////////////

const GenesisPairDiff & PairDataGenesis::gen_diff(int i) const {

   if(i < 0 || i > NPair) {
      mlog << Error << "\nPairBase::gen_diff() -> "
           << "range check error: " << i << " not in (0, "
           << NPair << ").\n\n";
      exit(1);
   }

   return(GenDiff[i]);
}

////////////////////////////////////////////////////////////////////////

bool PairDataGenesis::has_gen(const vector<const GenesisInfo *>& gi_list,
                              const GenesisInfo *gi, int &i) const {

   if(!gi) return(false);

   // Search for a match
   for(i=0; i<NPair; i++) {
      if(*(gi_list[i]) == *gi) return(true);
   }

   // No match
   i = bad_data_int;

   return(false);
}

////////////////////////////////////////////////////////////////////////

bool PairDataGenesis::has_fcst_gen(const GenesisInfo *fgi, int &i) const {
   return(has_gen(FcstGen, fgi, i));
}

////////////////////////////////////////////////////////////////////////

bool PairDataGenesis::has_best_gen(const GenesisInfo *bgi, int &i) const {
   return(has_gen(BestGen, bgi, i));
}

////////////////////////////////////////////////////////////////////////

bool PairDataGenesis::has_case(const ConcatString &best_id,
                               const unixtime init_ut, int &i) const {

   for(i=0; i<NPair; i++) {
      if(BestStormId[i] == best_id.c_str() &&
         InitTime[i]    == init_ut) return(true);
   }

   // No match
   i = bad_data_int;

   return(false);
}

////////////////////////////////////////////////////////////////////////

void PairDataGenesis::add_fcst_gen(const GenesisInfo *fgi) {

   if(!fgi) return;

   GenesisPairDiff diff;

   // Add the unmatched forecast
   NPair++;
   BestStormId.add(na_str);
   InitTime.add(fgi->init());
   LeadTime.add(fgi->genesis_lead());
   FcstGen.push_back(fgi);
   BestGen.push_back((GenesisInfo *) 0);
   GenDiff.push_back(diff);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataGenesis::add_best_gen(const GenesisInfo *bgi,
        const int fcst_beg, const int fcst_end, const int init_add,
        const unixtime init_beg, const unixtime init_end,
        const TimeArray &init_inc, const TimeArray &init_exc,
        const NumArray &init_hour, const NumArray &lead) {

   if(!bgi) return;

   int i_case;
   GenesisPairDiff diff;
   unixtime init_ut;

   // Define opportunities to forecast this event
   unixtime init_start = bgi->genesis_time() - fcst_end;
   unixtime init_stop  = bgi->genesis_time() - fcst_beg;

   // Add unmatched pair for each forecast opportunity
   for(init_ut=init_start; init_ut<=init_stop; init_ut+=init_add) {

      // Check if this initialization time should be used
      if((init_beg     > 0 &&  init_beg >   init_ut)  ||
         (init_end     > 0 &&  init_beg <   init_ut)  ||
         (init_inc.n() > 0 && !init_inc.has(init_ut)) ||
         (init_exc.n() > 0 &&  init_exc.has(init_ut)))
         continue;

      // Check if this initialization hour and lead time should be used
      if((init_hour.n() > 0 && !init_hour.has(unix_to_sec_of_day(init_ut))) ||
         (lead.n()      > 0 && !lead.has(nint(bgi->genesis_time() - init_ut))))
         continue;

      // Check if this case already exists
      if(!has_case(bgi->storm_id(), init_ut, i_case)) {

         // Add a new unmatched pair
         NPair++;
         BestStormId.add(bgi->storm_id());
         InitTime.add(init_ut);
         LeadTime.add(bgi->genesis_time() - init_ut);
         FcstGen.push_back((GenesisInfo *) 0);
         BestGen.push_back(bgi);
         GenDiff.push_back(diff);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataGenesis::add_gen_pair(const GenesisInfo *fgi,
                                   const GenesisInfo *bgi) {

   if(!fgi || !bgi) return;

   int i_case;
   GenesisPairDiff diff;

   // Update an existing case
   if(has_case(bgi->storm_id(), fgi->init(), i_case)) {
      FcstGen[i_case] = fgi;
      GenDiff[i_case].clear();
   }
   // Add a new case
   else {
      NPair++;
      BestStormId.add(bgi->storm_id());
      InitTime.add(fgi->init());
      LeadTime.add(fgi->genesis_lead());
      FcstGen.push_back(fgi);
      BestGen.push_back(bgi);
      GenDiff.push_back(diff);
   }
   
   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataGenesis::set_gen_diff(int i, const GenesisPairDiff &diff) {

   if(i < 0 || i > NPair) {
      mlog << Error << "\nPairBase::set_gen_diff() -> "
           << "range check error: " << i << " not in (0, "
           << NPair << ").\n\n";
      exit(1);
   }

   GenDiff[i] = diff;

   return;
}

////////////////////////////////////////////////////////////////////////
