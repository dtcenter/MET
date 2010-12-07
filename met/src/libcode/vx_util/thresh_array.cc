// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
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
#include <cmath>

#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

static const int thresharray_alloc_inc = 10;

////////////////////////////////////////////////////////////////////////
//
//  Code for class ThreshArray
//
////////////////////////////////////////////////////////////////////////

ThreshArray::ThreshArray() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

ThreshArray::~ThreshArray() {

   clear();
}

////////////////////////////////////////////////////////////////////////

ThreshArray::ThreshArray(const ThreshArray & a) {

   init_from_scratch();

   assign(a);
}

////////////////////////////////////////////////////////////////////////

ThreshArray & ThreshArray::operator=(const ThreshArray & a) {

   if(this == &a) return(*this);

   assign(a);

   return(* this);
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::init_from_scratch() {

   t = (SingleThresh *) 0;

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::clear() {

   if(t) { delete [] t;  t = (SingleThresh *) 0; }

   Nelements = Nalloc = 0;

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::assign(const ThreshArray & a) {
   int j;

   clear();

   extend(a.Nelements);

   for(j=0; j<(a.Nelements); j++) add(a.t[j]);

   Nelements = a.Nelements;

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::dump(ostream & out, int depth) const {
   int j;
   char tmp_str[512];

   Indent prefix(depth);
   Indent prefix2(depth + 1);

   out << prefix << "Nelements = " << Nelements << "\n";
   out << prefix << "Nalloc    = " << Nalloc    << "\n";

   for(j=0; j<Nelements; j++) {
      t[j].get_str(tmp_str);
      out << prefix2 << "Element # " << j << " = \""
          << tmp_str << "\"\n";
   }

   out.flush();

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::extend(int n) {
   int j, k;

   if(n <= Nalloc) return;

   k = n/thresharray_alloc_inc;

   if(n%thresharray_alloc_inc) k++;

   n = k*thresharray_alloc_inc;

   SingleThresh *u = (SingleThresh *) 0;

   u = new SingleThresh [n];

   if(t) {
      for(j=0; j<Nelements; j++) u[j] = t[j];

      delete [] t; t = (SingleThresh *) 0;
   }

   t = u; u = (SingleThresh *) 0;

   Nalloc = n;

   return;
}

////////////////////////////////////////////////////////////////////////

SingleThresh ThreshArray::operator[](int n) const {

   if((n < 0) || (n >= Nelements)) {
      cerr << "\n\n  ThreshArray::operator[](int) const -> "
           << "range check error!\n\n";
      exit(1);

   }

   return(t[n]);
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::add(const SingleThresh &st) {

   extend(Nelements + 1);

   t[Nelements] = st;

   Nelements++;

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::add(const double thresh, const ThreshType type) {
   SingleThresh st;

   st.set(thresh, type);

   extend(Nelements + 1);

   t[Nelements] = st;

   Nelements++;

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::add(const char *thresh_str) {
   SingleThresh st;

   st.set(thresh_str);

   extend(Nelements + 1);

   t[Nelements] = st;

   Nelements++;

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::add(const ThreshArray & a) {
   int j;

   if(a.n_elements() == 0) return;

   extend(Nelements + a.n_elements());

   for(j=0; j<(a.n_elements()); j++) add(a[j]);

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::parse_thresh_str(const char *thresh_str) {
   char *line = (char *) 0;
   char *c    = (char *) 0;
   char *lp   = (char *) 0;
   const char delim [] = " ";
   const int n = strlen(thresh_str);

   line = new char [n + 1];
   memset(line, 0, n + 1);
   strcpy(line, thresh_str);

   lp = line;

   while((c = strtok(lp, delim)) != NULL ) {

      add(c);

      lp = (char *) 0;
   }

   if(line) { delete [] line; line = (char *) 0; }
   lp = (char *) 0;

   return;
}

////////////////////////////////////////////////////////////////////////

int ThreshArray::has(const SingleThresh &st) const {
   int index, status;

   status = has(st, index);

   return(status);
}

////////////////////////////////////////////////////////////////////////

int ThreshArray::has(const SingleThresh &st, int & index) const {
   int j;

   index = -1;

   if(Nelements == 0) return(0);

   for(j=0; j<Nelements; j++) {

      if(t[j] == st) { index = j; return(1); }
   }

   //
   // Not found
   //

   return(0);
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::get_str(const char *sep, char *str) const {
   int i;
   char junk[1024];
   ConcatString tmp_str;

   for(i=0; i<Nelements; i++) {
      t[i].get_str(junk);

      if(i==0) tmp_str << junk;
      else     tmp_str << sep << junk;
   }

   strcpy(str, tmp_str.text());

   return;
}

////////////////////////////////////////////////////////////////////////

void ThreshArray::get_abbr_str(const char *sep, char *str) const {
   int i;
   char junk[1024];
   ConcatString tmp_str;

   for(i=0; i<Nelements; i++) {
      t[i].get_abbr_str(junk);

      if(i==0) tmp_str << junk;
      else     tmp_str << sep << junk;
   }

   strcpy(str, tmp_str.text());

   return;
}

////////////////////////////////////////////////////////////////////////

int ThreshArray::check(double v) const {
   int i, bin;

   //
   // Check that the threshold values are monotonically increasing
   // and the threshold types are inequalities that remain the same
   //
   for(i=0; i<Nelements-1; i++) {

      if(t[i].thresh >  t[i+1].thresh ||
         t[i].type   != t[i+1].type   ||
         t[i].type   == thresh_eq     ||
         t[i].type   == thresh_ne) {

         cerr << "\n\n  ThreshArray::check(double) const -> "
              << "thresholds must be monotonically increasing and be of "
              << "the same inequality type (lt, le, gt, or ge)."
              << "\n\n" << flush;
         exit(1);
      }
   }

   // For < and <=, check thresholds left to right.
   if(t[0].type == thresh_lt || t[0].type == thresh_le) {

      for(i=0, bin=-1; i<Nelements; i++) {
         if(t[i].check(v)) {
            bin = i;
            break;
         }
      }
      if(bin == -1) bin = Nelements;
   }
   // For > and >=, check thresholds right to left.
   else {

      for(i=Nelements-1, bin=-1; i>=0; i--) {
         if(t[i].check(v)) {
            bin = i+1;
            break;
         }
      }
      if(bin == -1) bin = 0;
   }

   return(bin);
}

////////////////////////////////////////////////////////////////////////

