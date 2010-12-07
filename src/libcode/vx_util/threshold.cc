// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "threshold.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////
//
// Code for class SingleThresh
//
////////////////////////////////////////////////////////////////////////

SingleThresh::SingleThresh() {
   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

SingleThresh::~SingleThresh() {
   clear();
}

////////////////////////////////////////////////////////////////////////

SingleThresh::SingleThresh(const SingleThresh &c) {

   init_from_scratch();

   assign(c);
}

////////////////////////////////////////////////////////////////////////

SingleThresh & SingleThresh::operator=(const SingleThresh &c) {

   if(this == &c) return(*this);

   assign(c);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

int SingleThresh::operator==(const SingleThresh &st) {
   int match = 0;

   if(is_eq(thresh, st.thresh) &&
      type == st.type) match = 1;

   return(match);
}

////////////////////////////////////////////////////////////////////////

void SingleThresh::init_from_scratch() {

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void SingleThresh::clear() {

   thresh = 0.0;
   type   = thresh_na;

   return;
}

////////////////////////////////////////////////////////////////////////

void SingleThresh::assign(const SingleThresh &c) {

   thresh = c.thresh;
   type   = c.type;

   return;
}

////////////////////////////////////////////////////////////////////////

void SingleThresh::set(double t, ThreshType ind) {

   thresh = t;
   type   = ind;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Parse out the threshold information from the form:
// lt, le, eq, ne, gt, ge
// ... OR ...
// <, <=, =, !=, >, >=
//
////////////////////////////////////////////////////////////////////////

void SingleThresh::set(const char *str) {
   int i, offset, found;
   char test_type[3], test_abbr[3];

   //
   // Check for bad data
   //
   if(strcmp(str, na_str      ) == 0 ||
      strcmp(str, bad_data_str) == 0) {

      type = thresh_na;
      thresh = 0.0;
      return;
   }

   //
   // Convert the first two characaters to lower case
   //
   test_type[0] = tolower(str[0]);
   test_type[1] = tolower(str[1]);
   test_type[2] = '\0';

   strcpy(test_abbr, test_type);

   if(test_type[1] != '=') test_type[1] = '\0';

   //
   // Search for the type of thresholding to be performed
   //
   found = 0;
   for(i=0; i<n_thresh_type; i++) {

      //
      // Check for strings: na, <, <=, =, !=, >, >=
      //
      if(strcmp(test_type, thresh_type_str[i]) == 0) {
         type   = (ThreshType) i;
         offset = strlen(thresh_type_str[i]);
         found  = 1;
         break;
      }

      //
      // Check for strings: na, lt, le, eq, ne, gt, ge
      //
      else if(strcmp(test_abbr, thresh_abbr_str[i]) == 0) {

         type   = (ThreshType) i;
         offset = strlen(thresh_abbr_str[i]);
         found  = 1;
         break;
      }
   }

   if(!found) {
      cerr << "\n\nERROR: SingleThresh::set() -> "
           << "each threshold value must be preceeded by one of "
           << "\"lt, le, eq, ne, gt, ge\" or \"<, <=, =, !=, >, >=\" "
           << "to indicate the type of thresholding to be performed \""
           << str << "\".\n\n" << flush;
      exit(1);
   }

   //
   // Parse out the threshold value
   //
   thresh = atof(str+offset);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Construct a string to represent the threshold type and value
//
////////////////////////////////////////////////////////////////////////

void SingleThresh::get_str(char *str) const {

   get_str(str, 3);

   return;
}

////////////////////////////////////////////////////////////////////////

void SingleThresh::get_str(char *str, int precision) const {
   char fmt_str[512];

   if(type == thresh_na) {
      strcpy(str, na_str);
   }
   else {
      sprintf(fmt_str, "%s%i%s", "%s%.", precision, "f");
      sprintf(str, fmt_str, thresh_type_str[type], thresh);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Construct a string to represent the threshold type and value
//
////////////////////////////////////////////////////////////////////////

void SingleThresh::get_abbr_str(char *str) const {

   get_abbr_str(str, 3);

   return;
}

////////////////////////////////////////////////////////////////////////

void SingleThresh::get_abbr_str(char *str, int precision) const {
   char fmt_str[512];

   if(type == thresh_na) {
      strcpy(str, na_str);
   }
   else {
      sprintf(fmt_str, "%s%i%s", "%s%.", precision, "f");
      sprintf(str, fmt_str, thresh_abbr_str[type], thresh);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Check whether or not the value meets the threshold criteria.
// Return 1 if it does and 0 otherwise.
//
////////////////////////////////////////////////////////////////////////

int SingleThresh::check(double v) {
   int r = 0;

   // Type of thresholding
   switch(type) {

      case thresh_lt: // less than
         if(v < thresh)        r = 1;
         break;
      case thresh_le: // less than or equal to
         if(v <= thresh)       r = 1;
         break;
      case thresh_eq: // equal to
         if(is_eq(v, thresh))  r = 1;
         break;
      case thresh_ne: // not equal to
         if(!is_eq(v, thresh)) r = 1;
         break;
      case thresh_gt: // greater than
         if(v > thresh)        r = 1;
         break;
      case thresh_ge: // greater than or equal to
         if(v >= thresh)       r = 1;
         break;

      default:
         cerr << "\n\nERROR: SingleThresh::check() -> "
              << "unexpected threshold indicator value of "
              << type << ".\n\n" << flush;
         exit(1);
         break;
   }

   return(r);
}

////////////////////////////////////////////////////////////////////////
//
// Begin miscellaneous functions
//
////////////////////////////////////////////////////////////////////////

int check_threshold(double v, double t, int t_ind) {
   SingleThresh st;

   st.set(t, (ThreshType) t_ind);

   return(st.check(v));
}

////////////////////////////////////////////////////////////////////////
//
// End miscellaneous functions
//
////////////////////////////////////////////////////////////////////////
