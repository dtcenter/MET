// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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
#include "prob_rirw_info.h"
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

   // Erase the entire vectors
   ProbRIRW.erase(ProbRIRW.begin(), ProbRIRW.end());
   ProbGen.erase(ProbGen.begin(), ProbGen.end());

   return;
}

////////////////////////////////////////////////////////////////////////

void ProbInfoArray::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   int i;

   out << prefix << "ProbInfoArray:\n"
       << prefix << "NProbRIRW = " << n_prob_rirw() << "\n";

   for(i=0; i<ProbRIRW.size(); i++) {
      out << prefix << "ProbRIRW[" << i+1 << "]:\n";
      ProbRIRW[i].dump(out, indent_depth+1);
   }

   out << prefix << "NProbGen = " << n_prob_gen() << "\n";

   for(i=0; i<ProbGen.size(); i++) {
      out << prefix << "ProbGen[" << i+1 << "]:\n";
      ProbGen[i].dump(out, indent_depth+1);
   }

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbInfoArray::serialize() const {
   ConcatString s;

   s << "ProbInfoArray: "
     << "NProbRIRW = " << n_prob_rirw()
     << ", NProbGen = " << n_prob_gen();

   return(s);
}

////////////////////////////////////////////////////////////////////////

ConcatString ProbInfoArray::serialize_r(int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString s;
   int i;

   if(ProbRIRW.size() > 0 ) {
      s << prefix << serialize() << ", ProbRIRW:\n";
   }
   for(i=0; i<ProbRIRW.size(); i++) {
      s << ProbRIRW[i].serialize_r(i+1, indent_depth+1);
   }

   if(ProbGen.size() > 0 ) {
      s << prefix << serialize() << ", ProbGen:\n";
   }

   for(i=0; i<ProbGen.size(); i++) {
      s << ProbGen[i].serialize_r(i+1, indent_depth+1);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void ProbInfoArray::assign(const ProbInfoArray &p) {

   clear();

   ProbRIRW = p.ProbRIRW;
   ProbGen  = p.ProbGen;

   return;
}

////////////////////////////////////////////////////////////////////////

const ProbInfoBase * ProbInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= n_probs())) {
      mlog << Error << "\nProbInfoBase * ProbInfoArray::operator[] -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   // Get a base pointer to the n-th probability
   const ProbInfoBase *ptr;

   if(ProbRIRW.size() > 0 && n < ProbRIRW.size()) {
      ptr = &ProbRIRW[n];
   }
   else {
      n  -= ProbRIRW.size();
      ptr = &ProbGen[n];
   }

   return(ptr);
}

////////////////////////////////////////////////////////////////////////

ProbRIRWInfo & ProbInfoArray::prob_rirw(int n) {

   // Check range
   if((n < 0) || (n >= (int) ProbRIRW.size())) {
      mlog << Error << "\nProbRIRWInfo & ProbInfoArray::prob_rirw(int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(ProbRIRW[n]);
}

////////////////////////////////////////////////////////////////////////

ProbGenInfo & ProbInfoArray::prob_gen(int n) {

   // Check range
   if((n < 0) || (n >= (int) ProbGen.size())) {
      mlog << Error << "\nProbGenInfo & ProbInfoArray::prob_gen(int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(ProbGen[n]);
}

////////////////////////////////////////////////////////////////////////

int ProbInfoArray::n_technique() const {
   StringArray sa;

   // Count the number of unique technique names
   for(int i=0; i<ProbRIRW.size(); i++) {
      if(!sa.has(ProbRIRW[i].technique())) {
         sa.add(ProbRIRW[i].technique());
      }
   }
   for(int i=0; i<ProbGen.size(); i++) {
      if(!sa.has(ProbGen[i].technique())) {
         sa.add(ProbGen[i].technique());
      }
   }

   return(sa.n());
}

////////////////////////////////////////////////////////////////////////

bool ProbInfoArray::add(const ATCFProbLine &l, double dland, bool check_dup) {
   bool status = false;

   // Range check the probability value
   if(l.prob() < 0 || l.prob() > 100) {
      mlog << Debug(4)
           << "bool ProbInfoArray::add() -> "
           << "skipping probability value (" << l.prob()
           << ") outside of range (0, 100).\n";
      return(false);
   }

   // Store based on the input line type
   switch(l.type()) {

      case(ATCFLineType_ProbRI):

         // Add line to an existing entry
         if(ProbRIRW.size()  > 0 &&
            ProbRIRW[ProbRIRW.size()-1].add(l, dland, check_dup)) {
            status = true;
         }
         // Add a new entry
         else {
            ProbRIRWInfo ri;
            ri.add(l, dland, check_dup);
            ProbRIRW.push_back(ri);
            status = true;
         }
         break;

      case(ATCFLineType_ProbGN):

         // Add line to an existing entry
         if(ProbGen.size()  > 0 &&
            ProbGen[ProbGen.size()-1].add(l, dland, check_dup)) {
            status = true;
         }
         // Add a new entry
         else {
            ProbGenInfo gi;
            gi.add(l, dland, check_dup);

            // Check for the expected genesis type and predicted location
            if(gi.gen_or_dis() != "genFcst") {
               mlog << Debug(4)
                    << "bool ProbInfoArray::add() -> "
                    << "skipping ATCF " << atcflinetype_to_string(ATCFLineType_ProbGN)
                    << " line with non-genesis probability type ("
                    << gi.gen_or_dis() << " != genFcst).\n";
            }
            else if(is_bad_data(gi.lat()) || is_bad_data(gi.lon())) {
               mlog << Debug(4)
                    << "bool ProbInfoArray::add() -> "
                    << "skipping ATCF " << atcflinetype_to_string(ATCFLineType_ProbGN)
                    << " line with no predicted genesis location.\n";
            }
            else {
               ProbGen.push_back(gi);
               status = true;
            }
         }
         break;

      default:
         mlog << Warning << "\nbool ProbInfoArray::add() -> "
              << "unexpected ATCF line type ("
              << atcflinetype_to_string(l.type()) << ")\n\n";
         status = false;
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

void ProbInfoArray::add(const ProbRIRWInfo &rirw) {
   ProbRIRW.push_back(rirw);
   return;
}

////////////////////////////////////////////////////////////////////////

void ProbInfoArray::add(const ProbGenInfo &gi) {
   ProbGen.push_back(gi);
   return;
}

////////////////////////////////////////////////////////////////////////
