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

#include "pair_data_genesis.h"

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

   // Add the unmatched forecast
   NPair++;
   BestStormId.add("");
   InitTime.add(fgi->init());
   LeadTime.add(fgi->genesis_lead());
   FcstGen.push_back(fgi);
   BestGen.push_back((GenesisInfo *) 0);

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataGenesis::add_best_gen(const GenesisInfo *bgi,
                                   const int beg,
                                   const int end,
                                   const int inc) {

   if(!bgi) return;

   int i_case;

   // Define opportunities to forecast this event
   unixtime init_beg = bgi->genesis_time() - end;
   unixtime init_end = bgi->genesis_time() - beg;

   // Add unmatched pair for each forecast opportunity
   for(unixtime init_ut=init_beg; init_ut<=init_end; init_ut+=inc) {

      // Check if this case already exists
      if(!has_case(bgi->storm_id(), init_ut, i_case)) {

         // Add a new unmatched pair
         NPair++;
         BestStormId.add(bgi->storm_id());
         InitTime.add(init_ut);
         LeadTime.add(bgi->genesis_time() - init_ut);
         FcstGen.push_back((GenesisInfo *) 0);
         BestGen.push_back(bgi);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void PairDataGenesis::add_gen_pair(const GenesisInfo *fgi,
                                   const GenesisInfo *bgi) {

   if(!fgi || !bgi) return;

   int i_case;

   // Update an existing case
   if(has_case(bgi->storm_id(), fgi->init(), i_case)) {
      FcstGen[i_case] = fgi;
   }
   // Add a new case
   else {
      NPair++;
      BestStormId.add(bgi->storm_id());
      InitTime.add(fgi->init());
      LeadTime.add(fgi->genesis_lead());
      FcstGen.push_back(fgi);
      BestGen.push_back(bgi);
   }
   
   return;
}

////////////////////////////////////////////////////////////////////////
