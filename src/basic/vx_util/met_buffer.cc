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

#include "met_buffer.h"
#include "read_fortran_binary.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const int default_rec_pad_size = 4;

static const bool default_swap_endian = false;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetBuffer
   //


////////////////////////////////////////////////////////////////////////


MetBuffer::MetBuffer()

{

mb_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MetBuffer::~MetBuffer()

{

mb_clear();

}


////////////////////////////////////////////////////////////////////////


MetBuffer::MetBuffer(const MetBuffer & b)

{

mb_init_from_scratch();

mb_assign(b);

}


////////////////////////////////////////////////////////////////////////


MetBuffer & MetBuffer::operator=(const MetBuffer & b)

{

if ( this == &b )  return ( * this );

mb_assign(b);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void MetBuffer::mb_init_from_scratch()

{

Buf = 0;

Nalloc = Nbytes = (bigint) 0;

mb_clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MetBuffer::mb_clear()

{

if ( Buf )  { delete [] Buf;  Buf = 0; }

Nalloc = Nbytes = (bigint) 0;

RecPadSize = default_rec_pad_size;

SwapEndian = default_swap_endian;

return;

}


////////////////////////////////////////////////////////////////////////


MetBuffer::MetBuffer(int initial_size)

{

mb_init_from_scratch();

if ( initial_size < 0 )  {

   mlog << Error 
        << "MetBuffer::MetBuffer(int initial_size) -> bad initial size ... " << initial_size << "\n\n";

   exit ( 1 );

}

if ( initial_size == 0 )  return;

extend(initial_size);

return;

}


////////////////////////////////////////////////////////////////////////


void MetBuffer::mb_assign(const MetBuffer & b)

{

mb_clear();

if ( b.Nbytes == 0 )  return;

extend(b.Nbytes);

Nbytes = b.Nbytes;

RecPadSize = b.RecPadSize;
SwapEndian = b.SwapEndian;

memcpy(Buf, b.Buf, Nbytes);

return;

}


////////////////////////////////////////////////////////////////////////


void MetBuffer::extend(bigint bytes)

{

if ( bytes < 0 )  {

   mlog << Error
        << "MetBuffer::extend(int bytes) -> bad value ... " << bytes << "\n\n";

   exit ( 1 );

}

if ( bytes <= Nalloc )  return;

unsigned char * u = new unsigned char [bytes];

memset(u, 0, bytes);

if ( Buf )  {

   memcpy(u, Buf, bytes);

   delete [] Buf;  Buf = 0;

}

Buf = u;  u = 0;

Nbytes = 0;

Nalloc = bytes;



return;

}


////////////////////////////////////////////////////////////////////////


void MetBuffer::set_rec_pad_size(int n)

{

   //
   //  legitimate values are 4 or 8
   //

if ( (n != 4) && (n != 8) )  {

   mlog << Error << "\n\n  MetBuffer::set_rec_pad_size(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

RecPadSize = n;

return;

}


////////////////////////////////////////////////////////////////////////


void MetBuffer::set_swap_endian(bool tf)

{

SwapEndian = tf;

return;

}


////////////////////////////////////////////////////////////////////////


int MetBuffer::read(const int fd, const bigint bytes)

{

extend(bytes);

int n_read;

n_read = ::read(fd, Buf, bytes);

Nbytes = n_read;

if ( n_read < 0 )  return ( -1 );

return ( n_read );

}


////////////////////////////////////////////////////////////////////////


bool MetBuffer::read_fortran(const int fd)

{

long long s = peek_record_size(fd, RecPadSize, SwapEndian);

extend(s);

long long n_read;

n_read = ::read_fortran_binary(fd, Buf, Nalloc, RecPadSize, SwapEndian);

Nbytes = n_read;

return ( false );

}


////////////////////////////////////////////////////////////////////////


