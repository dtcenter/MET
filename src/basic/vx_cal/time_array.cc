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
#include <cmath>

#include "vx_cal.h"
#include "vx_log.h"


using namespace std;


////////////////////////////////////////////////////////////////////////


static int compare_unixtime(const void *, const void *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class TimeArray
   //


////////////////////////////////////////////////////////////////////////


TimeArray::TimeArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


TimeArray::~TimeArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


TimeArray::TimeArray(const TimeArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


TimeArray & TimeArray::operator=(const TimeArray & a)

{

if ( this == &a )  return *this;

assign(a);

return *this;

}


////////////////////////////////////////////////////////////////////////


bool TimeArray::operator==(const TimeArray & a) const

{

if ( Nelements != a.Nelements )  return false;

bool status = true;

for (int j=0; j<Nelements; ++j)  {

   if ( e[j] != a.e[j] )  {
      status = false;
      break;
   }
}

return status;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::init_from_scratch()

{

e = (unixtime*) nullptr;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::clear()

{

if ( e )  { delete [] e;  e = (unixtime *) nullptr; }

Nelements = Nalloc = 0;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::erase()

{

Nelements = 0;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::assign(const TimeArray & a)

{

clear();

if ( a.Nelements == 0 )  return;

extend(a.Nelements);

for (int j=0; j<(a.Nelements); ++j)  {

   e[j] = a.e[j];

}

Nelements = a.Nelements;

Sorted = a.Sorted;


return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::extend(int n, bool exact)


{

if ( Nalloc >= n )  return;

if ( ! exact )  {

   int k;

   k = n/time_array_alloc_inc;

   if ( n%time_array_alloc_inc )  ++k;

   n = k*time_array_alloc_inc;

}

unixtime * u = (unixtime *) nullptr;

u = new unixtime [n];

if ( !u )  {

   mlog << Error << "\nvoid TimeArray::extend(int) -> "
        << "memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, n*sizeof(unixtime));

if ( e )  {

   for (int j=0; j<Nelements; ++j)  {

      u[j] = e[j];

   }

   delete [] e;  e = (unixtime *) nullptr;

}

e = u; u = (unixtime *) nullptr;

Nalloc = n;


return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "Sorted    = " << (Sorted ? "true" : "false") << "\n";

for (int j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " = "
       << unix_to_yyyymmdd_hhmmss(e[j]) << "\n";

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


unixtime TimeArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   mlog << Error << "\nTimeArray::operator[](int) const -> "
       << "range check error\n\n";

   exit ( 1 );

}

return e[n];

}


////////////////////////////////////////////////////////////////////////


int TimeArray::has(unixtime u) const

{

   return ( index ( u ) >= 0 );

}


////////////////////////////////////////////////////////////////////////


int TimeArray::index(unixtime u) const

{

int match = -1;

for (int j=0; j<Nelements; ++j)  {

   if ( e[j] == u )  {  match = j;  break;  }

}

return match;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add(unixtime u)

{

extend(Nelements + 1, false);

e[Nelements++] = u;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add(const TimeArray & a)

{

extend(Nelements + a.Nelements);

for (int j=0; j<(a.Nelements); ++j)  {

   e[Nelements++] = a.e[j];

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add_const(unixtime u, int n)

{

extend(Nelements + n);

int j;

for (j=0; j<n; ++j)  {

   e[Nelements++] = u;

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add_css(const char *text)

{

StringArray sa;

sa.parse_css(text);

extend(Nelements + sa.n_elements());

for (int j=0; j<sa.n_elements(); j++)  {

  add(timestring_to_unix(sa[j].c_str()));

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::set(int n, unixtime u)

{

if ( (n < 0) || (n >= Nelements) )  {

   mlog << Error << "\nTimeArray::set(int, unixtime) -> "
        << "range check error\n\n";

   exit ( 1 );

}

e[n] = u;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


unixtime TimeArray::min() const

{

unixtime u;

if(Nelements == 0)  return bad_data_ll;

u = e[0];
for(int j=0; j<Nelements; j++) {
   if(e[j] < u) u = e[j];
}

return u;

}


////////////////////////////////////////////////////////////////////////


unixtime TimeArray::max() const

{

unixtime u;

if(Nelements == 0)  return bad_data_ll;

u = e[0];
for(int j=0; j<Nelements; j++) {
   if(e[j] > u) u = e[j];
}

return u;

}


////////////////////////////////////////////////////////////////////////


ConcatString TimeArray::serialize() const

{

   ConcatString s;

   if(n_elements() == 0) return s;

   s << e[0];
   for(int j=1; j<n_elements(); j++) s << " " << unix_to_yyyymmdd_hhmmss(e[j]);

   return s;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::sort_array()

{

if ( Nelements <= 1 )  return;

qsort(e, Nelements, sizeof(unixtime), compare_unixtime);

Sorted = true;

return;

}


////////////////////////////////////////////////////////////////////////
//
// For each set of equally spaced times return the beginning and ending
// times of the segment.
//
////////////////////////////////////////////////////////////////////////

void TimeArray::equal_dt(TimeArray &beg, TimeArray &end) const

{

// Initialize
end.clear();
beg.clear();

if ( Nelements == 0 )  return;

// Use first point to begin first segment
beg.add(e[0]);

int cur_dt;
int prv_dt = 0;
bool new_ts = true;

for(int i=1; i<Nelements; i++, prv_dt=cur_dt) {
   cur_dt = (int)(e[i] - e[i-1]);
   if(new_ts) {
      prv_dt = cur_dt;
      new_ts = false;
   }

   // When the time step changes, begin a new segment
   if(cur_dt != prv_dt) {
      end.add(e[i-1]);
      beg.add(e[i]);
      new_ts = true;
   }
}

// Use last point to end the last segment
end.add(e[Nelements - 1]);

return;

}


////////////////////////////////////////////////////////////////////////


TimeArray TimeArray::subset(int beg, int end) const

{

TimeArray subset_ta;

// Check bounds
if ( beg < 0 || beg >= Nelements ||
     end < 0 || end >= Nelements ||
     end < beg )  {
   mlog << Error << "\nTimeArray::subset(int, int) -> "
        << "range check error\n\n";
   exit ( 1 );
}

// Store subset
for(int i=beg; i<=end; i++) subset_ta.add(e[i]);

return subset_ta;

}

////////////////////////////////////////////////////////////////////////


int compare_unixtime(const void *p1, const void *p2)

{

const unixtime *a = (const unixtime *) p1;
const unixtime *b = (const unixtime *) p2;


if ( (*a) < (*b) )  return -1;

if ( (*a) > (*b) )  return 1;


return 0;

}


////////////////////////////////////////////////////////////////////////


ConcatString write_css(const TimeArray &ta)

{

ConcatString css;

for ( int i=0; i<ta.n(); ++i )  {

   css << (i == 0 ? "" : ",") << unix_to_yyyymmdd_hhmmss(ta[i]);

}

return css;

}


////////////////////////////////////////////////////////////////////////
