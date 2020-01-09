

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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "vx_util.h"
#include "vx_log.h"
#include "check_endian.h"

#include "shp_file.h"
#include "shapetype_to_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct ShpFileHeader
   //


////////////////////////////////////////////////////////////////////////


void ShpFileHeader::set(unsigned char * buf)

{

int    * i = (int *) buf;
double * d = (double *) (buf + 36);

handle_big_4    (buf);
handle_big_4    (buf + 24);
handle_little_4 (buf + 28);
handle_little_4 (buf + 32);

file_code      = i[0];

file_length_16 = i[6];
version        = i[7];
shape_type     = i[8];

file_length_bytes = 2*file_length_16;

if ( is_big_endian() )  {

   for (int j=0; j<8; ++j)  shuffle_8(d + j);

}

x_min = d[0];
y_min = d[1];

x_max = d[2];
y_max = d[3];

z_min = d[4];
z_max = d[5];

m_min = d[6];
m_max = d[7];

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ShpFileHeader::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "file_code      = " << file_code         << "\n";
out << prefix << "file_length    = " << file_length_bytes << "\n";
out << prefix << "version        = " << version           << "\n";
out << prefix << "shape_type     = " << shapetype_to_string((ShapeType) shape_type) << "\n";

out << prefix << "\n";

out << prefix << "(x_min, x_max) = (" << x_min << ", " << x_max << ")\n";
out << prefix << "(y_min, y_max) = (" << y_min << ", " << y_max << ")\n";
out << prefix << "(z_min, z_max) = (" << z_min << ", " << z_max << ")\n";
out << prefix << "(m_min, m_max) = (" << m_min << ", " << m_max << ")\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct ShpRecordHeader
   //


////////////////////////////////////////////////////////////////////////


void ShpRecordHeader::set(unsigned char * buf)

{

int * i = (int *) buf;

handle_big_4(buf);
handle_big_4(buf + 4);

record_number_1   = i[0];
content_length_16 = i[1];

record_number_0 = record_number_1 - 1;

content_length_bytes = 2*content_length_16;


return;

}


////////////////////////////////////////////////////////////////////////


void ShpRecordHeader::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "record_number  = " << record_number_0      << " (0-based)\n";
out << prefix << "content_length = " << content_length_bytes << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ShpFile
   //


////////////////////////////////////////////////////////////////////////


ShpFile::ShpFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ShpFile::~ShpFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


ShpFile::ShpFile(const ShpFile &)

{

mlog << Error
     << "\n\n  ShpFile::ShpFile(const ShpFile &) -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


ShpFile & ShpFile::operator=(const ShpFile &)

{

mlog << Error
     << "\n\n  ShpFile::operator=(const ShpFile &) -> should never be called!\n\n";

exit ( 1 );

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ShpFile::init_from_scratch()

{

fd = -1;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void ShpFile::close()

{

if ( fd >= 0 )  ::close(fd);

fd = -1;

memset(&Header, 0, sizeof(Header));

Filename.clear();

At_Eof = false;


return;

}


////////////////////////////////////////////////////////////////////////


const char * ShpFile::filename() const

{

if ( Filename.empty() )  return ( (const char *) 0 );

return ( Filename.text() );

}


////////////////////////////////////////////////////////////////////////


bool ShpFile::open(const char * path)

{

close();

if ( (fd = met_open(path, O_RDONLY)) < 0 )  {

   fd = -1;

   return ( false );

}

Filename = path;

   //
   //  read the file header
   //

int n_read;
unsigned char buf[shp_file_header_bytes];

if ( (n_read = ::read(fd, buf, shp_file_header_bytes)) != shp_file_header_bytes )  {

   mlog << Error
        << "\n\n  ShpFile::open(const char * path) -> trouble reading file header from shp file \""
        << path << "\"\n\n";

   // exit ( 1 );

   close();

   return ( false );

}

Header.set(buf);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


int ShpFile::position() const

{

if ( ! is_open() )  {

   mlog << Error
        << "\n\n  ShpFile::position() -> no file open!\n\n";

   exit ( 1 );

   // return ( false );

}


// if ( ! is_open() )  return ( -1 );

const int pos = ::lseek(fd, 0, SEEK_CUR);

if ( pos < 0 )  {

   mlog << Error
        << "\n\n  ShpFile::position() const -> lseek error ... "
        << strerror(errno) << "\n\n";

   exit ( 1 );

}


return ( pos );

}


////////////////////////////////////////////////////////////////////////


void ShpFile::lseek(int offset, int whence)

{


if ( ! is_open() )  {

   mlog << Error
        << "\n\n  ShpFile::lseek() -> no file open!\n\n";

   exit ( 1 );

   // return ( false );

}


if ( ::lseek(fd, offset, whence) < 0 )  {

   mlog << Error
        << "\n\n  ShpFile::lseek() -> lseek(2) failed! ... "
        << strerror(errno) << "\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


bool ShpFile::read(unsigned char * buf, int n_bytes)

{

if ( ! is_open() )  {

   mlog << Error
        << "\n\n  ShpFile::read() -> no file open!\n\n";

   exit ( 1 );

   // return ( false );

}

int n_read;

n_read = ::read(fd, buf, n_bytes);

if ( n_read < 0 )  {   //  some kind of error

   mlog << Error
        << "\n\n  ShpFile::read() -> read error on shp file \""
        << Filename << "\"\n\n";

   exit ( 1 );

}

   //

if ( n_read == 0 )  return ( false );

   //

if ( n_read == n_bytes )  return ( true );


return ( false );   //  gotta return something

}


////////////////////////////////////////////////////////////////////////


bool ShpFile::read_sb(SmartBuffer & buf, int n_bytes)

{

int n_read;

n_read = buf.read(fd, n_bytes);

if ( n_read == n_bytes )  return ( true );

if ( n_read == 0 )  {

   At_Eof = true;

   return ( false );

}

if ( n_read < 0 )  {   //  some kind of error

   mlog << Error
        << "\n\n  ShpFile::read_sb() -> read error on shp file \""
        << Filename << "\"\n\n";

   exit ( 1 );

}


return ( false );

}


////////////////////////////////////////////////////////////////////////






