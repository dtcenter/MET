// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
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
#include <map>
#include <utility>

#include "tc_stat_files.h"

#include "met_stats.h"
#include "vx_tc_util.h"
#include "vx_log.h"
#include "vx_util.h"
#include "vx_math.h"

using namespace std;


////////////////////////////////////////////////////////////////////////
//
// Code for class TCStatFiles
//
////////////////////////////////////////////////////////////////////////

TCStatFiles::TCStatFiles() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCStatFiles::~TCStatFiles() {

   clear();
}

////////////////////////////////////////////////////////////////////////

TCStatFiles::TCStatFiles(const TCStatFiles &j) {

   init_from_scratch();

   assign(j);
}

////////////////////////////////////////////////////////////////////////

void TCStatFiles::add_files(const StringArray &files) {

   for(int i=0; i<files.n(); i++) add(files[i].c_str());

   return;
}

////////////////////////////////////////////////////////////////////////

bool TCStatFiles::operator>>(TrackPairInfo &pair) {
   TCStatLine line;
   bool status;

   // Initialize
   pair.clear();

   // Read lines to the end of the track or file
   while((status = (*this >> line))) {

      // Skip header and non-TCMPR/TCDIAG lines
      if(line.is_header() ||
         (line.type() != TCStatLineType::TCMPR &&
          line.type() != TCStatLineType::TCDIAG)) continue;

      // Add the current point
      pair.add(line);

      // Break out of the loop at the end of the track
      if(atoi(line.get_item("TOTAL")) ==
         atoi(line.get_item("INDEX"))) {

         // Check for a trailing TCDIAG line
         if(CurLDF.peek_line(line)) {
            if(line.type() == TCStatLineType::TCDIAG) {
               pair.add(line);
               CurLDF >> line;
            }
         }

         break;
      }
   } // end while

   return true;
}

////////////////////////////////////////////////////////////////////////

bool TCStatFiles::operator>>(ProbRIRWPairInfo &pair) {
   TCStatLine line;
   bool status;

   // Initialize
   pair.clear();

   // Read next line
   while((status = (*this >> line))) {

      // Skip header and non-PROBRIRW lines
      if(line.is_header() || line.type() != TCStatLineType::ProbRIRW) continue;

      // Add the current point
      pair.set(line);

      break;

   } // end while

   return status;
}

////////////////////////////////////////////////////////////////////////

bool TCStatFiles::operator>>(TCStatLine &line) {
   bool status;

   // Read next line
   while((status = (*this >> line))) {

      // Skip header and invalid line types
      if(line.is_header() || line.type() == TCStatLineType::None) continue;

      break;

   } // end while

   return status;
}

////////////////////////////////////////////////////////////////////////

