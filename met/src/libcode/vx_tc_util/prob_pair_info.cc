// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "prob_pair_info_base.h"
#include "prob_rirw_pair_info.h"
#include "prob_pair_info_array.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class ProbPairInfoArray
//
////////////////////////////////////////////////////////////////////////

ProbPairInfoArray::ProbPairInfoArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ProbPairInfoArray::~ProbPairInfoArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ProbPairInfoArray::ProbPairInfoArray(const ProbPairInfoArray & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

ProbPairInfoArray & ProbPairInfoArray::operator=(const ProbPairInfoArray & t) {

   if(this == &t) return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ProbPairInfoArray::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbPairInfoArray::clear() {

   // Deallocate memory for each entry
   for(int i=0; i<Pair.size(); i++) {
      if(Pair[i]) { delete (Pair[i]); (Pair[i]) = (ProbPairInfoBase *) 0; }
   }

   // Erase the entire vector
   Pair.erase(Pair.begin(), Pair.end());

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbPairInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "ProbPairInfoArray:\n"
       << prefix << "NPairs = "   << n_pairs() << "\n";

   for(i=0; i<Pair.size(); i++) {
      out << prefix << "Pair[" << i+1 << "]:\n";
      Pair[i]->dump(out, indent_depth+1);
   }

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbPairInfoArray::serialize() const {
   ConcatString s;

   s << "ProbPairInfoArray: "
     << "NPairs = " << n_pairs();

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbPairInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;

   s << prefix << serialize() << ", Pairs:\n";

   for(int i=0; i<Pair.size(); i++) {
      s << Pair[i]->serialize_r(i+1, indent_depth+1);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbPairInfoArray::assign(const ProbPairInfoArray &p) {

   clear();

   // Allocate space and copy each element
   for(int i=0; i<p.Pair.size(); i++) {
      Pair.push_back(new_prob_pair(p.Pair[i]->type()));
      *(Pair[i]) = *(p.Pair[i]);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

const ProbPairInfoBase * ProbPairInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= Pair.size())) {
      mlog << Error
           << "\nProbPairInfoArray::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(Pair[n]);
}

////////////////////////////////////////////////////////////////////////

bool ProbPairInfoArray::add(const ATCFProbLine &l, bool check_dup) {

   // Check for no pairs or a mismatch with the latest entry
   if( Pair.size() == 0 ||
      (Pair.size()  > 0 && !Pair[Pair.size()-1]->add(l, check_dup))) {

      // Allocate and store new pair
      Pair.push_back(new_prob_pair(l.type()));

      // Add current line to new pair
      if(!Pair[Pair.size()-1]->add(l, check_dup)) {
         mlog << Warning
              << "\nbool ProbPairInfoArray::add(const ATCFProbLine &l, bool check_dup) -> "
              << "cannot store ATCFProbLine:\n" << l.get_line() << "\n\n";
         return(false);
      }
   }

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool ProbPairInfoArray::add(const ProbPairInfoBase *p) {
   ProbPairInfoBase *new_pair;

   // Allocate new pair, set its value, and store it
   new_pair = new_prob_pair(p->type());
   *(new_pair) = *(p);
   Pair.push_back(new_pair);

   return(true);
}

////////////////////////////////////////////////////////////////////////

ProbPairInfoBase * new_prob_pair(const ATCFLineType t) {
   ProbPairInfoBase *new_pair = (ProbPairInfoBase *) 0;

   switch(t) {
      case ATCFLineType_ProbRIRW:
         new_pair = new ProbRIRWPairInfo;
         break;

      default:
         mlog << Warning
              << "\nProbPairInfoBase * new_prob_pair(const ATCFLineType t) -> "
              << "unexpected ATCF line type ("
              << atcflinetype_to_string(t) << ")\n\n";
         break;
   }

   if(!new_pair) {
      mlog << Error
           << "\nProbPairInfoBase * new_prob_pair(const ATCFLineType t) -> "
           << "memory allocation error!\n\n";
      exit(1);
   }

   return(new_pair);
}

////////////////////////////////////////////////////////////////////////
