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
#include <zlib.h>

#include "vx_log.h"
#include "flate_filter.h"


////////////////////////////////////////////////////////////////////////


static const int buf_size = 65536;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class FlateEncodeFilter
   //


////////////////////////////////////////////////////////////////////////


FlateEncodeFilter::FlateEncodeFilter()

{

inbuf  = new unsigned char [buf_size];
outbuf = new unsigned char [buf_size];

s = new z_stream;

if ( !inbuf || !outbuf || !s )  {

   mlog << Error 
        << "\n\n  FlateEncodeFilter::FlateEncodeFilter() -> memory allocation error\n\n";

   exit ( 1 );

}

memset(s, 0, sizeof(*s));

s->zalloc = Z_NULL;
s->zfree  = Z_NULL;
s->opaque = Z_NULL;

if ( deflateInit(s, Z_BEST_COMPRESSION) != Z_OK )  {

   mlog << Error
        << "\n\n  FlateEncodeFilter::FlateEncodeFilter() -> can't initialize the z_stream\n\n";

   exit ( 1 );

}

flush_mode = Z_NO_FLUSH;

inbytes = 0;

}


////////////////////////////////////////////////////////////////////////


FlateEncodeFilter::~FlateEncodeFilter()

{

if (  inbuf )  { delete []  inbuf;   inbuf = (unsigned char *) 0; }
if ( outbuf )  { delete [] outbuf;  outbuf = (unsigned char *) 0; }

if ( s )  { delete s;  s = (z_stream *) 0; }

inbytes = 0;

flush_mode = Z_NO_FLUSH;

}


////////////////////////////////////////////////////////////////////////


void FlateEncodeFilter::eat(unsigned char c)

{

   //
   //  is there room in the input buffer for this byte?
   //

if ( inbytes < buf_size )  {

   inbuf[inbytes++] = c;

   return;

}

   //
   //  if not, then hand the input buffer to the deflate function ...
   //

s->avail_in = buf_size;

s->next_in = inbuf;

do_output();

inbytes = 0;


   //
   //  ... and then add the byte to the buffer
   //

inbuf[inbytes++] = c;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void FlateEncodeFilter::eod()

{

   //
   //  finish the data processing
   //

s->avail_in = (uInt) inbytes;

s->next_in = inbuf;

flush_mode = Z_FINISH;

if ( inbytes > 0 )  do_output();

inbytes = 0;

   //
   //  finish up the zlib stuff
   //

(void) deflateEnd(s);

   //
   //  send the "end of data" signal to the next filter down the line
   //

next->eod();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void FlateEncodeFilter::do_output()

{

int j;
int status;
int bytes;


do {

   s->avail_out = buf_size;

   s->next_out = outbuf;

   status = deflate(s, flush_mode);

   if ( status == Z_STREAM_ERROR )  {

      mlog << Error
           << "\n\n  FlateEncodeFilter::do_output() -> runtime error\n\n";

      (void) deflateEnd(s);

      exit ( 1 );

   }

   bytes = buf_size - s->avail_out;

   for (j=0; j<bytes; ++j)  {

      next->eat(outbuf[j]);

   }

} while ( bytes > 0 );


if ( (flush_mode == Z_FINISH) && (status != Z_STREAM_END) )  {

   mlog << Error
        << "\n\n  FlateEncodeFilter::do_output() -> bad status at end of operations\n\n";

   (void) deflateEnd(s);

   exit ( 1 );

}



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////



