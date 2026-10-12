////////////////////////////////////////////////////////////////////////

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"

#include "smart_buffer.h"

using namespace std;


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

if ( this == &b )  return *this;

assign(b);

return *this;

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::clear()

{

Buf.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::assign(const SmartBuffer & b)

{

Buf = b.Buf;

return;

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::extend(const int bytes)

{

if ( (int) Buf.size() >= bytes )  return;

   //
   //  resize, not reserve: callers write into the buffer through the
   //  unsigned char * conversion immediately after extending it
   //

Buf.resize(bytes, 0);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int SmartBuffer::read (const int fd, const int bytes)

{

int n_read;

if ( bytes > size() )  extend(bytes);

n_read = ::read(fd, Buf.data(), bytes);

   //
   //  done
   //

return n_read;

}


////////////////////////////////////////////////////////////////////////


int SmartBuffer::write (const int fd, const int bytes) const

{

int n_written;

if ( bytes > size() )  {

   mlog << Error
        << "\n\n  SmartBuffer:: write () -> can't write more than "
        << size() << " bytes!\n\n";

   exit ( 1 );

}

n_written = ::write(fd, Buf.data(), bytes);

   //
   //  done
   //

return n_written;

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::read_from_buf (void * other_buf, const int bytes, const int pos)

{

if ( pos + bytes > size() )  extend(pos + bytes);

memcpy(Buf.data() + pos, other_buf, bytes);


return;

}


////////////////////////////////////////////////////////////////////////


void SmartBuffer::write_to_buf   (void * other_buf, const int bytes, const int pos) const

{

if ( pos + bytes > size() )  {

   mlog << Error
        << "\n\n  SmartBuffer::write_to_buf() -> can't write values past end of buffer\n\n";

   exit ( 1 );

}

memcpy(other_buf, Buf.data() + pos, bytes);

return;

}


////////////////////////////////////////////////////////////////////////



