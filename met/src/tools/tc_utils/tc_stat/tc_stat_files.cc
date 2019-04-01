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
#include <map>
#include <utility>

#include "tc_stat_files.h"

#include "met_stats.h"
#include "vx_tc_util.h"
#include "vx_log.h"
#include "vx_util.h"
#include "vx_math.h"

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

TCStatFiles & TCStatFiles::operator=(const TCStatFiles &j) {

   if(this == &j) return(*this);

   assign(j);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCStatFiles::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatFiles::clear() {

   FileList.clear();

   CurFile = -1;

   CurLDF.close();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatFiles::assign(const TCStatFiles & j) {

   clear();

   FileList = j.FileList;

   rewind();

   return;
}


////////////////////////////////////////////////////////////////////////

void TCStatFiles::add_files(const StringArray &files) {

   FileList.add(files);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatFiles::rewind() {

   CurFile = -1;

   CurLDF.close();

   return;
}


////////////////////////////////////////////////////////////////////////

bool TCStatFiles::operator>>(TrackPairInfo &pair) {
   TCStatLine line;

   // Initialize
   pair.clear();

   // Check the status of the current file
   if(!CurLDF.ok()) {

      // Increment the file index
      CurFile++;

      // Check for the last file
      if(CurFile == FileList.n_elements()) return(false);
      else {

         // Open the next file for reading
         CurLDF.close();
         if(!(CurLDF.open(FileList[CurFile].c_str()))) {
            mlog << Error << "\nTCStatFiles::operator>>(TrackPairInfo &) -> "
                 << "can't open file \"" << FileList[CurFile]
                 << "\" for reading\n\n";
            exit(1);
         }

         // List file being read
         mlog << Debug(3)
              << "Reading file " << CurFile+1 << " of "
              << FileList.n_elements() << ": " << FileList[CurFile]
              << "\n";

      } // end else
   } // end if

   // Read lines to the end of the track or file
   while(CurLDF >> line) {

      // Skip header and non-TCMPR lines
      if(line.is_header() || line.type() != TCStatLineType_TCMPR) continue;

      // Add the current point
      pair.add(line);

      // Break out of the loop at the end of the track
      if(atoi(line.get_item("TOTAL")) ==
         atoi(line.get_item("INDEX"))) break;

   } // end while

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool TCStatFiles::operator>>(ProbRIRWPairInfo &pair) {
   TCStatLine line;
   bool status;

   // Initialize
   pair.clear();

   // Check the status of the current file
   if(!CurLDF.ok()) {

      // Increment the file index
      CurFile++;

      // Check for the last file
      if(CurFile == FileList.n_elements()) return(false);
      else {

         // Open the next file for reading
         CurLDF.close();
         if(!(CurLDF.open(FileList[CurFile].c_str()))) {
            mlog << Error << "\nTCStatFiles::operator>>(ProbRIRWPairInfo &) -> "
                 << "can't open file \"" << FileList[CurFile]
                 << "\" for reading\n\n";
            exit(1);
         }

         // List file being read
         mlog << Debug(3)
              << "Reading file " << CurFile+1 << " of "
              << FileList.n_elements() << ": " << FileList[CurFile]
              << "\n";

      } // end else
   } // end if

   // Read next line
   while((status = (CurLDF >> line))) {

      // Skip header and non-PROBRIRW lines
      if(line.is_header() || line.type() != TCStatLineType_ProbRIRW) continue;

      // Add the current point
      pair.set(line);

      break;

   } //end while

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool TCStatFiles::operator>>(TCStatLine &line) {
   bool status;

   // Check the status of the current file
   if(!CurLDF.ok()) {

      // Increment the file index
      CurFile++;

      // Check for the last file
      if(CurFile == FileList.n_elements()) return(false);
      else {

         // Open the next file for reading
         CurLDF.close();
         if(!(CurLDF.open(FileList[CurFile].c_str()))) {
            mlog << Error << "\nTCStatFiles::operator>>(TCStatLine &) -> "
                 << "can't open file \"" << FileList[CurFile]
                 << "\" for reading\n\n";
            exit(1);
         }

         // List file being read
         mlog << Debug(3)
              << "Reading file " << CurFile+1 << " of "
              << FileList.n_elements() << ": " << FileList[CurFile]
              << "\n";

      } // end else
   } // end if

   // Read next line
   while((status = (CurLDF >> line))) {

      // Skip header and invalid line types
      if(line.is_header() || line.type() == NoTCStatLineType) continue;

      break;

   } //end while

   return(status);
}

////////////////////////////////////////////////////////////////////////
