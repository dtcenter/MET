// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_math.h"

#include "atcf_prob_line.h"
#include "atcf_offsets.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
//
//  Code for class ATCFProbLine
//
////////////////////////////////////////////////////////////////////////

ATCFProbLine::ATCFProbLine() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ATCFProbLine::~ATCFProbLine() {
   clear();
}

////////////////////////////////////////////////////////////////////////

ATCFProbLine::ATCFProbLine(const ATCFProbLine &l) {
   init_from_scratch();

   assign(l);
}

////////////////////////////////////////////////////////////////////////

ATCFProbLine & ATCFProbLine::operator=(const ATCFProbLine &l) {

   if(this == &l) return(*this);

   assign(l);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void ATCFProbLine::init_from_scratch() {

   ATCFLineBase::init_from_scratch();

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFProbLine::assign(const ATCFProbLine &l) {

   clear();

   ATCFLineBase::assign(l);

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFProbLine::dump(ostream &out, int indent_depth) const {
   Indent prefix(indent_depth);
   ConcatString cs;

   ATCFLineBase::dump(out, indent_depth);

   out << prefix << "Prob            = " << prob() << "\n";
   out << prefix << "ProbItem        = " << prob_item() << "\n";

   out << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

void ATCFProbLine::clear() {

   ATCFLineBase::clear();

   return;
}

////////////////////////////////////////////////////////////////////////

int ATCFProbLine::read_line(LineDataFile * ldf) {
   int status = 0;
   int n_expect;

   // Read lines until good status is found
   while(status == 0) {

      clear();

      // Return bad status from the base class
      if(!(status = ATCFLineBase::read_line(ldf))) return(0);

      // Check the line type
      switch(Type) {
         case ATCFLineType_ProbRI:
            n_expect = MinATCFProbRIRWElements;
            break;

         case ATCFLineType_ProbGN:
            n_expect = MinATCFProbGNElements;
            break;

         default:
            mlog << Debug(10)
                 << "ATCFProbLine::read_line(LineDataFile * ldf) -> "
                 << "skipping ATCF line type ("
                 << atcflinetype_to_string(Type) << ")\n";
            status = 0;
            continue;
      }

      // Check for the minumum number of elements
      if(n_items() < n_expect) {
         mlog << Warning
              << "\nint ATCFProbLine::read_line(LineDataFile * ldf) -> "
              << "found fewer than the expected number of elements ("
              << n_items() << "<" << n_expect
              << ") in ATCF " << atcflinetype_to_string(Type) << " line:\n"
              << DataLine::get_line() << "\n\n";
         status = 0;
         continue;
      }
	}

   return(1);
}

////////////////////////////////////////////////////////////////////////

int ATCFProbLine::prob() const {
   return(parse_int(get_item(ProbOffset).c_str()));
}

////////////////////////////////////////////////////////////////////////

int ATCFProbLine::prob_item() const {
   return(parse_int(get_item(ProbItemOffset).c_str()));
}

////////////////////////////////////////////////////////////////////////
