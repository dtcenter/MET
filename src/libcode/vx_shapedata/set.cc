// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   set.cc
//
//   Description:
//
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    04-15-05  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "set.h"
#include "vx_log.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
//  Code for class FcstObsSet
//
///////////////////////////////////////////////////////////////////////////////

FcstObsSet::FcstObsSet() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

FcstObsSet::~FcstObsSet() {
   clear();
}

///////////////////////////////////////////////////////////////////////////////

FcstObsSet::FcstObsSet(const FcstObsSet & s) {
   assign(s);
}

///////////////////////////////////////////////////////////////////////////////

FcstObsSet & FcstObsSet::operator=(const FcstObsSet & s) {

   if(this == &s) return *this;

   assign(s);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

void FcstObsSet::clear() {

   fcst_number.clear();
   obs_number.clear();

   n_fcst = 0;
   n_obs = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void FcstObsSet::assign(const FcstObsSet & s) {

   fcst_number = s.fcst_number;
   obs_number = s.obs_number;

   n_fcst = s.n_fcst;
   n_obs  = s.n_obs;

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool FcstObsSet::has_fcst(int k) const {

   for(int j=0; j<n_fcst; j++) {
      if(fcst_number[j] == k) return true;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

bool FcstObsSet::has_obs(int k) const {

   for(int j=0; j<n_obs; j++) {
      if(obs_number[j] == k) return true; 
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

void FcstObsSet::add_pair(int fcst, int obs) {

   if(fcst >= 0) add_fcst(fcst);
   if(obs  >= 0) add_obs(obs);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void FcstObsSet::add_fcst(int k) {

   if(has_fcst(k)) return;

   fcst_number.emplace_back(k);

   n_fcst++;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void FcstObsSet::add_obs(int k) {

   if(has_obs(k)) return;

   obs_number.emplace_back(k);

   n_obs++;

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
// Code for class SetCollection
//
///////////////////////////////////////////////////////////////////////////////

SetCollection::SetCollection() {
   init_from_scratch();
}

///////////////////////////////////////////////////////////////////////////////

SetCollection::~SetCollection() {
   all_clear();
}

///////////////////////////////////////////////////////////////////////////////

SetCollection::SetCollection(const SetCollection & a) {
   init_from_scratch();
   assign(a);
}

///////////////////////////////////////////////////////////////////////////////

SetCollection & SetCollection::operator=(const SetCollection & a) {

   if(this == &a) return *this;

   assign(a);

   return *this;
}

///////////////////////////////////////////////////////////////////////////////

void SetCollection::init_from_scratch() {

   set = nullptr;

   all_clear();

   extend(10);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void SetCollection::clear() {

   n_sets  = 0;

   for(int j=0; j<n_alloc; j++) set[j].clear();

   return;
}

///////////////////////////////////////////////////////////////////////////////

void SetCollection::all_clear() {

   if(set) { delete [] set; set = nullptr; }

   n_sets  = 0;

   n_alloc = 0;

   return;
}

///////////////////////////////////////////////////////////////////////////////


void SetCollection::assign(const SetCollection & s) {

   all_clear();

   if(!(s.set)) return;

   extend(s.n_alloc);

   n_sets = s.n_sets;

   for(int j=0; j<n_sets; j++) set[j] = s.set[j];

   return;
}

///////////////////////////////////////////////////////////////////////////////

void SetCollection::extend(int N) {

   if(N <= n_alloc)  return;

   int k = N/set_alloc_inc;

   if(N%set_alloc_inc) k++;

   k *= set_alloc_inc;

   auto *u = new FcstObsSet [k];

   if(set) {
      for(int j=0; j<n_alloc; j++) u[j] = set[j];
      delete [] set; set = nullptr;
   }

   set = u; u = nullptr;

   n_alloc = k;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void SetCollection::add_pair(int fcst, int obs) {

   extend(n_sets + 1);

   set[n_sets].clear();

   set[n_sets].add_pair(fcst, obs);

   n_sets++;

   bool need_merge;
   do {
      need_merge = merge();
   } while(need_merge);

   return;
}

///////////////////////////////////////////////////////////////////////////////

bool SetCollection::merge() {

   if(n_sets <= 1) return false;

   bool need_merge = false;

   int jm=0;
   int km=0;
   for(int j=0; j<(n_sets - 1); j++) {
      for(int k=(j + 1); k<n_sets; k++) {
         if(!need_merge && fcst_obs_sets_overlap(set[j], set[k])) {
            need_merge = true;
            jm = j;
            km = k;
         }
      }
   }

   if(need_merge) merge_two(jm, km);

   return need_merge;
}

///////////////////////////////////////////////////////////////////////////////

void SetCollection::merge_two(int index1, int index2) {

   int imin = (index1 < index2) ? index1 : index2;
   int imax = (index1 > index2) ? index1 : index2;

   FcstObsSet a(union_fcst_obs_sets(set[imin], set[imax]));

   for(int j=imax; j<(n_sets - 1); j++) {
      set[j] = set[j + 1];
   }

   n_sets--;

   for(int j=imin; j<(n_sets - 1); j++) {
      set[j] = set[j + 1];
   }

   n_sets--;

   set[n_sets] = a;

   n_sets++;

   for(int j=n_sets; j<n_alloc; j++) {
      set[j].clear();
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

int SetCollection::fcst_set_number(int fcst_number) const {

   for(int j=0; j<n_sets; j++) {
      if(set[j].has_fcst(fcst_number)) return j;
   }

   return -1;
}

///////////////////////////////////////////////////////////////////////////////

int SetCollection::obs_set_number(int obs_number) const {

   for(int j=0; j<n_sets; j++) {
      if(set[j].has_obs(obs_number)) return j;
   }

   return -1;
}

///////////////////////////////////////////////////////////////////////////////

bool SetCollection::is_fcst_matched(int fcst_number) const {

   bool matched = false;

   //
   // Find the set number containing this fcst object
   //
   int j = fcst_set_number(fcst_number);

   //
   // Check to see if the set contains obs objects
   //
   if(j != -1 && set[j].n_obs > 0) matched = true;

   return matched;
}

///////////////////////////////////////////////////////////////////////////////

bool SetCollection::is_obs_matched(int obs_number) const {

   bool matched = false;

   //
   // Find the set number containing this obs object
   //
   int j = obs_set_number(obs_number);

   //
   // Check to see if the set contains fcst objects
   //
   if(j != -1 && set[j].n_fcst > 0) matched = true;

   return matched;
}

///////////////////////////////////////////////////////////////////////////////
//
// Eliminate any sets with either n_fcst or n_obs = 0
//
///////////////////////////////////////////////////////////////////////////////

void SetCollection::clear_empty_sets() {

   for(int i=0; i<n_sets; i++) {
      if(set[i].n_fcst == 0 || set[i].n_obs == 0) {
         for(int j=i; (j+1)<n_sets; j++) set[j] = set[j+1];
         n_sets--;
         i--;
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Code for misc functions
//
///////////////////////////////////////////////////////////////////////////////

bool fcst_obs_sets_overlap(const FcstObsSet &a, const FcstObsSet &b) {

   //
   // Check fcst's
   //
   for(int j=0; j<(a.n_fcst); j++) {
      if(b.has_fcst(a.fcst_number[j])) return true;
   }

   //
   // Check obs's
   //
   for(int j=0; j<(a.n_obs); j++) {
      if(b.has_obs(a.obs_number[j])) return true;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

FcstObsSet union_fcst_obs_sets(const FcstObsSet &a, const FcstObsSet &b) {
   int j;
   FcstObsSet c;

   for(j=0; j<(a.n_fcst); j++) {
      c.add_fcst(a.fcst_number[j]);
   }

   for(j=0; j<(b.n_fcst); j++) {
      c.add_fcst(b.fcst_number[j]);
   }

   for(j=0; j<(a.n_obs); j++) {
      c.add_obs(a.obs_number[j]);
   }

   for(j=0; j<(b.n_obs); j++) {
      c.add_obs(b.obs_number[j]);
   }

   return c;
}

///////////////////////////////////////////////////////////////////////////////

ostream & operator<<(ostream &out, const FcstObsSet &set) {

   //
   // Write the fcst's
   //
   out.setf(ios::fixed);

   out << "n_fcst = ";

   out.width(2);  out << (set.n_fcst);

   out << " ... ";

   for(int j=0; j<(set.n_fcst); j++) {
      out.width(3);   out << (set.fcst_number[j]) << " ";
   }

   out << "\n";

   //
   // Write the obs's
   //
   out.setf(ios::fixed);

   out << "n_obs = ";

   out.width(2);  out << (set.n_obs);

   out << " ... ";

   for(int j=0; j<(set.n_obs); j++) {
      out.width(3);   out << (set.obs_number[j]) << " ";
   }

   out << "\n\n";

   return out;
}

///////////////////////////////////////////////////////////////////////////////

ostream & operator<<(ostream &out, const SetCollection &c) {

   for(int j=0; j<(c.n_sets); j++) {
      out << j << "\n";
      out << (c.set[j]);
   }

   return out;
}

///////////////////////////////////////////////////////////////////////////////
