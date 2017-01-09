// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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

#include "prob_info_base.h"
#include "prob_ri_info.h"
#include "prob_info_array.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ProbInfoArray
//
////////////////////////////////////////////////////////////////////////

ProbInfoArray::ProbInfoArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ProbInfoArray::~ProbInfoArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ProbInfoArray::ProbInfoArray(const ProbInfoArray & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ProbInfoArray & ProbInfoArray::operator=(const ProbInfoArray & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ProbInfoArray::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbInfoArray::clear() {

   // Erase the entire vector
   ProbRI.erase(ProbRI.begin(), ProbRI.end());

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "ProbInfoArray:\n"
       << prefix << "NProbRI = "   << n_prob_ri() << "\n";

   for(i=0; i<ProbRI.size(); i++) {
      out << prefix << "ProbRI[" << i+1 << "]:\n";
      ProbRI[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbInfoArray::serialize() const {
   ConcatString s;

   s << "ProbInfoArray: "
     << "NProbRI = " << n_prob_ri();

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;

   s << prefix << serialize() << ", ProbRI:\n";

   for(int i=0; i<ProbRI.size(); i++) {
      s << ProbRI[i].serialize_r(i+1, indent_depth+1);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbInfoArray::assign(const ProbInfoArray &p) {

   clear();

   // Allocate space and copy each element
   for(int i=0; i<p.ProbRI.size(); i++) {
      ProbRI.push_back(p.ProbRI[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

const ProbInfoBase * ProbInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= n_probs())) {
      mlog << Error
           << "\nProbInfoBase * ProbInfoArray::get_probri(int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   // Return a base pointer to the n-th probability
   // Need to revise for each new probability vector

   return(&ProbRI[n]);
}

////////////////////////////////////////////////////////////////////////

const ProbRIInfo & ProbInfoArray::prob_ri(int n) const {

   // Check range
   if((n < 0) || (n >= ProbRI.size())) {
      mlog << Error
           << "\nProbRIInfo & ProbInfoArray::prob_ri(int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(ProbRI[n]);
}

////////////////////////////////////////////////////////////////////////

bool ProbInfoArray::add(const ATCFProbLine &l, bool check_dup) {

   // Range check the probability value
   if(l.prob() < 0 || l.prob() > 100) {
      mlog << Warning
           << "\nbool ProbInfoArray::add() -> "
           << "bad probability value ... \"" << l.prob() << "\"\n\n";
      return(false);
   }

   // Store based on the input line type
   switch(l.type()) {

      case(ATCFLineType_ProbRI):

         // Check for no entries or a mismatch with the latest entry
         if( ProbRI.size() == 0 ||
            (ProbRI.size()  > 0 &&
            !ProbRI[ProbRI.size()-1].add(l, check_dup))) {

            // Store new entry
            ProbRIInfo ri;
            ri.add(l, check_dup);
            ProbRI.push_back(ri);
         }
         break;

      default:
         mlog << Warning
              << "\nbool ProbInfoArray::add() -> "
              << "unexpected ATCF line type ("
              << atcflinetype_to_string(l.type()) << ")\n\n";
         return(false);
         break;
   }

   return(true);
}

////////////////////////////////////////////////////////////////////////

void ProbInfoArray::add(const ProbRIInfo &ri) {
   ProbRI.push_back(ri);
   return;
}

////////////////////////////////////////////////////////////////////////
