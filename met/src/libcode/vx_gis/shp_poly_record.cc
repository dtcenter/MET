

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
#include <string.h>
#include <cmath>

#include "vx_util.h"
#include "vx_log.h"
#include "smart_buffer.h"
#include "check_endian.h"

#include "shp_file.h"
#include "shp_poly_record.h"
#include "shapetype_to_string.h"


////////////////////////////////////////////////////////////////////////


static SmartBuffer Buf;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct ShpPolyRecord
   //


////////////////////////////////////////////////////////////////////////


void ShpPolyRecord::set(unsigned char * buf)

{

int j;
int * i = (int *) buf;
int bytes, offset;


handle_little_4(i);

shape_type = i[0];

if ( shape_type != shape_type_polygon )  {

   mlog << Error
        << "\n\n  ShpPolyRecord::set(unsigned char *) -> bad shape type ... "
        << shapetype_to_string((ShapeType) shape_type) << "\n\n";

   exit ( 1 );

}

   //
   //  since the double "bbox" is at offset 4 bytes into the buffer, 
   //
   //    alignment considerations prevent us from doing a simple cast 
   //
   //    like "(double *) (buf + 4)"
   //

for (j=0; j<4; ++j)  {

   handle_little_8( bbox + 4 + 8*j );

}

memcpy(bbox, buf + 4, 4*sizeof(double));

i = (int *) (buf + 36);

handle_little_4(i);
handle_little_4(i + 4);

n_parts = i[0];

n_points = i[1];

   //
   //  read parts array
   //

bytes = n_parts*sizeof(int);

parts.extend(n_parts);

memcpy(parts.buf(), buf + 44, bytes);

if ( is_big_endian() )  {

   i = parts.buf();

   for (j=0; j<n_parts; ++j)  shuffle_4(i + j);

}

parts.set_n_elements(n_parts);

   //
   //  read points array
   //

offset = 44 + bytes;

bytes = n_points*2*sizeof(double);

points.extend(n_points);

memcpy(points.buf(), buf + offset, bytes);

if ( is_big_endian() )  {

   double * d = (double *) (points.buf());

   for (j=0; j<(2*n_points); ++j)  shuffle_8(d + j);

}

points.set_n_elements(n_points);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ShpPolyRecord::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);
Indent p2(depth + 1);


out << prefix << "shape_type = " << shapetype_to_string((ShapeType) shape_type) << "\n";

out << prefix << "\n";

out << prefix << "x_min      = " << x_min() << "\n";
out << prefix << "x_max      = " << x_max() << "\n";

out << prefix << "\n";

out << prefix << "y_min      = " << y_min() << "\n";
out << prefix << "y_max      = " << y_max() << "\n";

out << prefix << "\n";

out << prefix << "n_parts    = " << n_parts  << "\n";
out << prefix << "n_points   = " << n_points << "\n";

out << prefix << "\n";

out << prefix << "parts      = [ \n";

for (j=0; j<n_parts; ++j)  {

   out << p2 << parts[j] << '\n';

}

out << prefix << "]\n";

out << prefix << "points     = [ \n";

for (j=0; j<n_points; ++j)  {

   out << p2 << '(' << points[j].x << ", " << points[j].y << ")\n";

}

out << prefix << "]\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


double ShpPolyRecord::lat(int k) const

{

if ( (k < 0) || (k >= n_points) )  {

   mlog << Error
        << "\n\n  ShpPolyRecord::lat(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( points[k].y );

}


////////////////////////////////////////////////////////////////////////


double ShpPolyRecord::lon(int k) const

{

if ( (k < 0) || (k >= n_points) )  {

   mlog << Error
        << "\n\n  ShpPolyRecord::lon(int) const -> range check error\n\n";

   exit ( 1 );

}

double t = points[k].x;

return ( -t );

}


////////////////////////////////////////////////////////////////////////


int ShpPolyRecord::start_index(int partno) const

{

if ( (partno < 0) || (partno >= n_parts) )  {

   mlog << Error
        << "\n\n  ShpPolyRecord::start_index(int) const -> range check error\n\n";

   exit ( 1 );


}

return ( parts[partno] );

}


////////////////////////////////////////////////////////////////////////


int ShpPolyRecord::stop_index(int partno) const

{

if ( (partno < 0) || (partno >= n_parts) )  {

   mlog << Error
        << "\n\n  ShpPolyRecord::stop_index(int) const -> range check error\n\n";

   exit ( 1 );


}

if ( partno == (n_parts - 1) )  return ( n_points - 1 );

return ( parts[partno + 1] - 1 );

}


////////////////////////////////////////////////////////////////////////


void ShpPolyRecord::toggle_longitudes()

{

int j;
double * d = (double *) points.buf();


for (j=0; j<n_points; ++j)  {

   *d = -(*d);

   d += 2;   

}


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


bool operator>>(ShpFile & file, ShpPolyRecord & record)

{

unsigned char rh_buf[shp_record_header_bytes];


   //
   //  read the record header
   //

if ( ! file.read(rh_buf, shp_record_header_bytes) )  {

   if ( file.at_eof() )  return ( false );

   mlog << Error
        << "\n\n  operator>>(ShpFile &, ShpPolyRecord &) -> trouble reading record header from shp file \""
        << (file.filename()) << "\"\n\n";

   exit ( 1 );

}

record.rh.set(rh_buf);


   //
   //  read the record
   //

Buf.extend(record.rh.content_length_bytes);

// if ( record.rh.content_length_bytes >= buf_size )  {
// 
//    mlog << Error
//         << "\n\n  operator>>(ShpFile &, ShpPolyRecord &) -> buffer too small ... increase to at least "
//         << (record.rh.content_length_bytes) << "\n\n";
// 
//    exit ( 1 );
// 
// }

if ( ! file.read(Buf, record.rh.content_length_bytes) )  {

   mlog << Error
        << "\n\n  operator>>(ShpFile &, ShpPolyRecord &) -> read error in shp file \""
        << (file.filename()) << "\"\n\n";

   exit ( 1 );

}

record.set((unsigned char *) Buf);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////





