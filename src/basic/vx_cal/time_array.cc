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
#include <cmath>

#include "vx_cal.h"
#include "vx_log.h"


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

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void TimeArray::init_from_scratch()

{

e = (unixtime*) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::clear()

{

if ( e )  { delete [] e;  e = (unixtime *) 0; }

Nelements = Nalloc = 0;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::assign(const TimeArray & a)

{

clear();

if ( a.Nelements == 0 )  return;

extend(a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   e[j] = a.e[j];

}

Nelements = a.Nelements;

Sorted = a.Sorted;


return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::extend(int n)

{

if ( Nalloc >= n )  return;

int k;

k = n/time_array_alloc_inc;

if ( n%time_array_alloc_inc )  ++k;

n = k*time_array_alloc_inc;

unixtime * u = (unixtime *) 0;

u = new unixtime [n];

if ( !u )  {

   mlog << Error << "\nvoid TimeArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

int j;

memset(u, 0, n*sizeof(unixtime));

if ( e )  {

   for (j=0; j<Nelements; ++j)  {

      u[j] = e[j];

   }

   delete [] e;  e = (unixtime *) 0;

}

e = u; u = (unixtime *) 0;

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

int j;

for (j=0; j<Nelements; ++j)  {

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

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


int TimeArray::has(unixtime u) const

{

   return ( index ( u ) >= 0 );

}


////////////////////////////////////////////////////////////////////////


int TimeArray::index(unixtime u) const

{

int j, match = -1;

for (j=0; j<Nelements; ++j)  {

   if ( e[j] == u )  {  match = j;  break;  }

}

return ( match );

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add(unixtime u)

{

extend(Nelements + 1);

e[Nelements++] = u;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::add(const TimeArray & a)

{

extend(Nelements + a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   e[Nelements++] = a.e[j];

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

int j;

for (j=0; j<sa.n_elements(); j++)  {

  add(timestring_to_unix(sa[j].c_str()));

}

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TimeArray::set(int n, unixtime u)

{

if ( (n < 0) || (n >= Nelements) )  {

   mlog << Error << "\nTimeArray::set(int, unixtime) -> range check error\n\n";

   exit ( 1 );

}

e[n] = u;

Sorted = false;

return;

}


////////////////////////////////////////////////////////////////////////


unixtime TimeArray::min() const

{

int j;
unixtime u;

if(Nelements == 0)  return(bad_data_ll);

for(j=0, u=e[0]; j<Nelements; j++) {
   if(e[j] < u) u = e[j];
}

return(u);

}


////////////////////////////////////////////////////////////////////////


unixtime TimeArray::max() const

{

int j;
unixtime u;

if(Nelements == 0)  return(bad_data_ll);

for(j=0, u=e[0]; j<Nelements; j++) {
   if(e[j] > u) u = e[j];
}

return(u);

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

int i, cur_dt, prv_dt;
bool new_ts;

// Initialize
end.clear();
beg.clear();

if ( Nelements == 0 )  return;

// Use first point to begin first segment
beg.add(e[0]);

for(i=1, new_ts=true; i<Nelements; i++, prv_dt=cur_dt) {
   cur_dt = e[i] - e[i-1];
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

return ( subset_ta );

}

////////////////////////////////////////////////////////////////////////


int compare_unixtime(const void *p1, const void *p2)

{

const unixtime *a = (const unixtime *) p1;
const unixtime *b = (const unixtime *) p2;


if ( (*a) < (*b) )  return ( -1 );

if ( (*a) > (*b) )  return (  1 );


return ( 0 );

}


////////////////////////////////////////////////////////////////////////
