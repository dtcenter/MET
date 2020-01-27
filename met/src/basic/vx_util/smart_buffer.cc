

////////////////////////////////////////////////////////////////////////


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
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"

#include "smart_buffer.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SmartBuffer
   //


////////////////////////////////////////////////////////////////////////


SmartBuffer::SmartBuffer()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SmartBuffer::~SmartBuffer()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SmartBuffer::SmartBuffer(const SmartBuffer & b)

{

init_from_scratch();

assign(b);

}


////////////////////////////////////////////////////////////////////////


SmartBuffer & SmartBuffer::operator=(const SmartBuffer & b)

{

if ( this == &b )  return ( * this );

assign(b);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::clear()

{

if ( Buf )  { delete [] Buf;  Buf = 0; }

Size = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::init_from_scratch()

{

Buf = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::assign(const SmartBuffer & b)

{

clear();

if ( b.Size == 0 )  return;

Buf = new unsigned char [b.Size];

memcpy(Buf, b.Buf, b.Size);

Size = b.Size;

return;

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::extend(const int bytes)

{

if ( Size >= bytes )  return;

unsigned char * u = 0;

u = new unsigned char [bytes];

   //
   //  If there is any old data, copy it to the new buffer
   //

if ( Buf )  {

   if ( Size > 0 )  memcpy(u, Buf, Size);

   delete [] Buf;  Buf = 0;

}

   //
   //  At this point, Buf is zero, whether there was any old data or not
   //

Buf = u;  u = 0;   //  handoff

Size = bytes;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int SmartBuffer::read (const int fd, const int bytes)

{

int n_read;

if ( bytes > Size )  extend(bytes);

n_read = ::read(fd, Buf, bytes);

   //
   //  done
   //

return ( n_read );

}


////////////////////////////////////////////////////////////////////////


int SmartBuffer::write (const int fd, const int bytes) const

{

int n_written;

if ( bytes > Size )  {

   mlog << Error
        << "\n\n  SmartBuffer:: write () -> can't write more than "
        << Size << " bytes!\n\n";

   exit ( 1 );

}

n_written = ::write(fd, Buf, bytes);

   //
   //  done
   //

return ( n_written );

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::read_from_buf (void * other_buf, const int bytes, const int pos)

{

if ( pos + bytes > Size )  extend(pos + bytes);

memcpy(Buf + pos, other_buf, bytes);


return;

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::write_to_buf   (void * other_buf, const int bytes, const int pos) const

{

if ( pos + bytes > Size )  {

   mlog << Error
        << "\n\n  SmartBuffer::write_to_buf() -> can't write values past end of buffer\n\n";

   exit ( 1 );

}

memcpy(other_buf, Buf + pos, bytes);

return;

}


////////////////////////////////////////////////////////////////////////



