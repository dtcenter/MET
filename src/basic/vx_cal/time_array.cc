// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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


   //
   //  Code for class TimeArray
   //


////////////////////////////////////////////////////////////////////////


TimeArray::~TimeArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


TimeArray::TimeArray(const TimeArray & a)

{

assign(a);

}


////////////////////////////////////////////////////////////////////////


TimeArray::TimeArray(TimeArray && a) noexcept
   : e(move(a.e)), Sorted(a.Sorted)
{

a.Sorted = false;

}

////////////////////////////////////////////////////////////////////////


TimeArray & TimeArray::operator=(const TimeArray & a)

{

if ( this == &a )  return *this;

assign(a);

return *this;

}


////////////////////////////////////////////////////////////////////////


TimeArray & TimeArray::operator=(TimeArray && a) noexcept

{

if ( this == &a )  return *this;

e = move(a.e);
Sorted = a.Sorted;
a.Sorted = false;

return *this;

}


////////////////////////////////////////////////////////////////////////


bool TimeArray::operator==(const TimeArray & a) const

{

if ( n() != a.n() )  return false;

bool status = true;

for (int j=0; j<n(); ++j)  {

   if ( e[j] != a.e[j] )  {
      status = false;
      break;
   }
}

return status;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::clear()

{

e.clear();

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::erase()

{

int n = n_elements();
e.clear();
e.reserve(n);

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::assign(const TimeArray & a)

{

clear();

e = a.e;

Sorted = a.Sorted;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::extend(int len)


{

e.reserve(len);

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Nelements = " << n() << "\n";
out << prefix << "Sorted    = " << (Sorted ? "true" : "false") << "\n";

for (int j=0; j<n(); ++j)  {

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

if ( (n < 0) || (n >= n_elements()) )  {

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

for (int j=0; j<n(); ++j)  {

   if ( e[j] == u )  {  match = j;  break;  }

}

return match;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add(unixtime u)

{

e.emplace_back(u);

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add(const TimeArray & a)

{

extend(n() + a.n());

for (int j=0; j<(a.n()); ++j)  {

   e.emplace_back(a.e[j]);

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add_const(unixtime u, int n)

{

extend(n_elements() + n);

for (int j=0; j<n; ++j)  {

   e.emplace_back(u);

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add_css(const char *text)

{

StringArray sa;

sa.parse_css(text);

extend(n_elements() + sa.n());

for (int j=0; j<sa.n(); j++)  {

  add(timestring_to_unix(sa[j].c_str()));

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::set(int n, unixtime u)

{

if ( (n < 0) || (n >= n_elements()) )  {

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

if(e.empty())  return bad_data_ll;

unixtime u = e[0];
for(int j=0; j<n(); j++) {
   if(e[j] < u) u = e[j];
}

return u;

}


////////////////////////////////////////////////////////////////////////


unixtime TimeArray::max() const

{

if(e.empty())  return bad_data_ll;

unixtime u = e[0];
for(int j=0; j<n(); j++) {
   if(e[j] > u) u = e[j];
}

return u;

}


////////////////////////////////////////////////////////////////////////


ConcatString TimeArray::serialize() const

{

ConcatString s;

if(e.empty()) return s;

s << e[0];
for(int j=1; j<n(); j++) {
   s << " " << unix_to_yyyymmdd_hhmmss(e[j]);
}

return s;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::sort_array()

{

if ( n() <= 1 )  return;

sort(e.begin(), e.end());

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

if ( e.empty() )  return;

// Use first point to begin first segment
beg.add(e[0]);

int cur_dt;
int prv_dt = 0;
bool new_ts = true;

for(int i=1; i<n(); i++, prv_dt=cur_dt) {
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
end.add(e[n() - 1]);

return;

}


////////////////////////////////////////////////////////////////////////


TimeArray TimeArray::subset(int beg, int end) const

{

TimeArray subset_ta;

// Check bounds
if ( beg < 0 || beg >= n() ||
     end < 0 || end >= n() ||
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


ConcatString write_css(const TimeArray &ta)

{

ConcatString css;

for ( int i=0; i<ta.n(); ++i )  {

   css << (i == 0 ? "" : ",") << unix_to_yyyymmdd_hhmmss(ta[i]);

}

return css;

}


////////////////////////////////////////////////////////////////////////
