// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <string.h>

#include "vx_log.h"
#include "rle_filter.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class RunLengthEncodeFilter
   //


////////////////////////////////////////////////////////////////////////


RunLengthEncodeFilter::RunLengthEncodeFilter()

{

mode = start;

q.clear();

}


////////////////////////////////////////////////////////////////////////


RunLengthEncodeFilter::~RunLengthEncodeFilter() { }


////////////////////////////////////////////////////////////////////////


void RunLengthEncodeFilter::eat(unsigned char u)

{

switch ( mode )  {

   case start:
      q.enqueue(u);
      mode = literal;
      break;

   case run:
      run_eat(u);
      break;

   case literal:
      literal_eat(u);
      break;

   default:
      mlog << Error << "\nRunLengthEncodeFilter::eat(unsigned char) -> bad mode\n\n";
      exit ( 1 );
      break;

}

return;

}


////////////////////////////////////////////////////////////////////////


void RunLengthEncodeFilter::run_eat(unsigned char u)

{

   //
   //  are we still on a run?
   //

if ( u == q.last_char() )   {

   q.enqueue(u);

   if ( q.run_count() >= 128 )  {

      dump_run(q.run_count());

      mode = start;

   }

   return;

}

   //
   //  run finished
   //

dump_run(q.run_count());

mode = start;

eat(u);

   //
   //
   //

return;

}


////////////////////////////////////////////////////////////////////////


void RunLengthEncodeFilter::literal_eat(unsigned char u)

{


if ( q.n_elements() >= 128 )  {

   dump_literal(q.n_elements());

   mode = start;

   eat(u);

   return;

}

   //
   //  q has less than 128 elements
   //


q.enqueue(u);

if ( q.run_count() >= rle_enough )  {

   dump_literal(q.n_elements() - q.run_count());

   mode = run;

}

return;

}


////////////////////////////////////////////////////////////////////////


void RunLengthEncodeFilter::dump_literal(int length)

{

if ( length == 0 )  return;

int j, bytes;
unsigned char u;

while ( length > 0 )  {

   bytes = length;

   if ( bytes > 128 )  bytes = 128;

   u = (unsigned char) (bytes - 1);

   next->eat(u);

   for (j=0; j<bytes; ++j)  {

      u = q.dequeue();

      next->eat(u);

   }

   length -= bytes;

}


return;

}


////////////////////////////////////////////////////////////////////////


void RunLengthEncodeFilter::dump_run(int length)

{

if ( (length < 2) || (length > 128) )  {

   mlog << Error << "\nRunLengthEncodeFilter::do_run() -> bad length ... " << length << "\n\n";

   exit ( 1 );

}

int j;
unsigned char u;

   //
   //  Note:  2 <= length <= 128 implies that
   //
   //         129 <= (257 - length) <= 255
   //

u = (unsigned char) (257 - length);

next->eat(u);

// u = (unsigned char) last_char;
u = q.dequeue();

next->eat(u);

   //
   //  remove the run of bytes from the queue
   //

for (j=0; j<(length - 1); ++j)  {

   u = q.dequeue();

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void RunLengthEncodeFilter::eod()

{

if ( q.n_elements() > 0 )  dump_literal(q.n_elements());

next->eat((unsigned char) 128);

next->eod();

return;

}


////////////////////////////////////////////////////////////////////////





