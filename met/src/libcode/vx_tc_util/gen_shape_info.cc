// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2021
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

#include "gen_shape_info.h"

////////////////////////////////////////////////////////////////////////
//
//  Code for class GenShapeInfo
//
////////////////////////////////////////////////////////////////////////

GenShapeInfo::GenShapeInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

GenShapeInfo::~GenShapeInfo() {
   clear();
}

////////////////////////////////////////////////////////////////////////

GenShapeInfo::GenShapeInfo(const GenShapeInfo &g) {
   clear();
   assign(g);
}

////////////////////////////////////////////////////////////////////////

GenShapeInfo & GenShapeInfo::operator=(const GenShapeInfo &g) {

   if(this == &g) return(*this);

   assign(g);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void GenShapeInfo::clear() {

   Basin.clear();

   FileTime  = (unixtime) 0;
   IssueTime = (unixtime) 0;

   Poly.clear();
   LeadSec.clear();
   ProbVal.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString GenShapeInfo::serialize() const {
   ConcatString s;

   s << "GenShapeInfo: "
     << "Basin = \"" << Basin << "\""
     << ", FileTime = \"" << (FileTime > 0 ?
           unix_to_yyyymmdd_hhmmss(FileTime).text() : na_str) << "\""
     << ", IssueTime = \"" << (IssueTime > 0 ?
           unix_to_yyyymmdd_hhmmss(IssueTime).text() : na_str) << "\""
     << ", NPoints = " << Poly.n_points
     << ", Lat = " << Poly.y_min() << " to " << Poly.y_max()
     << ", Lon = " << Poly.x_min() << " to " << Poly.x_max();

   return(s);

}

////////////////////////////////////////////////////////////////////////

void GenShapeInfo::assign(const GenShapeInfo &gsi) {

   clear();

   Basin = gsi.Basin;

   FileTime  = gsi.FileTime;
   IssueTime = gsi.IssueTime;

   Poly    = gsi.Poly;
   LeadSec = gsi.LeadSec;
   ProbVal = gsi.ProbVal;

   return;
}

////////////////////////////////////////////////////////////////////////

void GenShapeInfo::set_basin(const char *s) {
   Basin = s;
   return;
}

////////////////////////////////////////////////////////////////////////

void GenShapeInfo::set_time(unixtime file_ut) {

   FileTime = file_ut;

   // Round the file timestamp to the nearest synoptic time (00, 06, 12, 18)
   int h3 = 3*sec_per_hour;
   int h6 = 6*sec_per_hour;
   IssueTime = ( (int) (file_ut + h3) / h6 ) * h6;

   return;
}

////////////////////////////////////////////////////////////////////////

void GenShapeInfo::set_poly(const ShpPolyRecord &rec) {
   Poly = rec;
   return;
}

////////////////////////////////////////////////////////////////////////

void GenShapeInfo::add_prob(int sec, double prob) {

   // Check range of values
   if(sec < 0 || prob < 0.0 || prob > 1.0) {
      mlog << Error << "\nGenShapeInfo::add_prob() -> "
           << "unexpected lead time (" << sec
           << ") or probability value (" << prob
           << ")!\n\n";
      exit(1);
   }

   LeadSec.add(sec);
   ProbVal.add(prob);

   return;
}

////////////////////////////////////////////////////////////////////////

const ShpPolyRecord &GenShapeInfo::poly() const {
   return(Poly);
}

////////////////////////////////////////////////////////////////////////

double GenShapeInfo::center_lat() const {
   return((Poly.y_min() + Poly.y_max())/2.0);
}

////////////////////////////////////////////////////////////////////////

double GenShapeInfo::center_lon() const {
   return((Poly.x_min() + Poly.x_max())/2.0);
}

////////////////////////////////////////////////////////////////////////

int GenShapeInfo::n_prob() const {
   return(ProbVal.n());
}

////////////////////////////////////////////////////////////////////////

int GenShapeInfo::lead_sec(int i) const {

   // Check range of values
   if(i < 0 || i > LeadSec.n()) {
      mlog << Error << "\nGenShapeInfo::lead_sec(int) -> "
           << "range check error (" << i << ")!\n\n";
      exit(1);
   }

   return(LeadSec[i]);
}

////////////////////////////////////////////////////////////////////////

double GenShapeInfo::prob_val(int i) const {

   // Check range of values
   if(i < 0 || i > ProbVal.n()) {
      mlog << Error << "\nGenShapeInfo::prob_val(int) -> "
           << "range check error (" << i << ")!\n\n";
      exit(1);
   }

   return(ProbVal[i]);
}

////////////////////////////////////////////////////////////////////////
//
//  Code for class GenShapeInfoArray
//
////////////////////////////////////////////////////////////////////////

GenShapeInfoArray::GenShapeInfoArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

GenShapeInfoArray::~GenShapeInfoArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

GenShapeInfoArray::GenShapeInfoArray(const GenShapeInfoArray & t) {

   init_from_scratch();

   assign(t);
}

////////////////////////////////////////////////////////////////////////

GenShapeInfoArray & GenShapeInfoArray::operator=(const GenShapeInfoArray & t) {

   if(this == &t)  return(*this);

   assign(t);

   return(*this);
}

////////////////////////////////////////////////////////////////////////
//
// Define equality as duplicates with the same file time
//
////////////////////////////////////////////////////////////////////////

bool GenShapeInfo::operator==(const GenShapeInfo & gsi) const {

   return(is_duplicate(gsi) && FileTime == gsi.FileTime);
}


////////////////////////////////////////////////////////////////////////
//
// Duplicates have the same basin, issue time, number of points,
// and range of lat/lons
//
////////////////////////////////////////////////////////////////////////

bool GenShapeInfo::is_duplicate(const GenShapeInfo & gsi) const {

   return(Basin         == gsi.Basin         &&
          IssueTime     == gsi.IssueTime     &&
          Poly.n_points == gsi.Poly.n_points &&
          Poly.x_min()  == gsi.Poly.x_min()  &&
          Poly.x_max()  == gsi.Poly.x_max()  &&
          Poly.y_min()  == gsi.Poly.y_min()  &&
          Poly.y_max()  == gsi.Poly.y_max());
}


////////////////////////////////////////////////////////////////////////

void GenShapeInfoArray::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GenShapeInfoArray::clear() {

   GenShape.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void GenShapeInfoArray::assign(const GenShapeInfoArray &ga) {

   clear();

   for(int i=0; i<ga.GenShape.size(); i++) add(ga[i]);

   return;
}

////////////////////////////////////////////////////////////////////////

const GenShapeInfo & GenShapeInfoArray::operator[](int n) const {

   // Check range
   if((n < 0) || (n >= GenShape.size())) {
      mlog << Error << "\nGenShapeInfoArray::operator[](int) -> "
           << "range check error for index value " << n << "\n\n";
      exit(1);
   }

   return(GenShape[n]);
}

////////////////////////////////////////////////////////////////////////

bool GenShapeInfoArray::add(const GenShapeInfo &gsi, bool check_dup) {

   // Check for duplicates:
   //   - Skip exact duplicates
   //   - Replace older duplicates with newer ones
   //   - Skip older duplicates
   if(check_dup) {

      // Loop over existing shapes
      for(int i=0; i<GenShape.size(); i++) {

         // Handle duplicates
         if(GenShape[i].is_duplicate(gsi)) {

            // Exact duplicates have the same file time stamp
            if(GenShape[i].file_time() == gsi.file_time()) {
               mlog << Debug(5) << "Skip exact duplicate "
                    << gsi.serialize() << "\n";
               return(false);
            }
            // Replace older duplicates with more recent file time stamps
            else if(GenShape[i].file_time() < gsi.file_time()) {
               mlog << Debug(5)
                    << "Replace older (FileTime = \""
                    << unix_to_yyyymmdd_hhmmss(GenShape[i].file_time())
                    << "\") with newer " << gsi.serialize() << "\n";
               GenShape[i] = gsi;
               return(false);
            }
            else {
               mlog << Debug(5) << "Skip older duplicate "
                    << gsi.serialize() << "\n";
               return(false);
            }
         }
      }
   } // end if

   // Store the genesis shape object
   GenShape.push_back(gsi);

   return(true);
}

////////////////////////////////////////////////////////////////////////

bool GenShapeInfoArray::has(const GenShapeInfo &gsi) const {

   for(int i=0; i<GenShape.size(); i++) {
      if(gsi == GenShape[i]) return(true);
   }

   return(false);
}

////////////////////////////////////////////////////////////////////////
