

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"

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


shuffle_4(buf);
shuffle_4(buf + 24);
// shuffle_4(buf + 28);
// shuffle_4(buf + 32);

file_code      = i[0];

file_length_16 = i[6];
version        = i[7];
shape_type     = i[8];

file_length_bytes = 2*file_length_16;

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

out << prefix << "file_code   = " << file_code         << "\n";
out << prefix << "file_length = " << file_length_bytes << "\n";
out << prefix << "version     = " << version           << "\n";
out << prefix << "shape_type  = " << shapetype_to_string((ShapeType) shape_type) << "\n";

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

shuffle_4(buf);
shuffle_4(buf + 4);

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


void ShpPolylineRecord::set(unsigned char * buf)

{

// int j;
int * i = (int *) buf;
int bytes, offset;


shape_type = i[0];

if ( shape_type != shape_type_polyline )  {

   cerr << "\n\n  ShpPolylineRecord::set(unsigned char *) -> bad shape type ... "
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

memcpy(bbox, buf + 4, 4*sizeof(double));

// for (j=0; j<4; ++j)  {
// 
//    shuffle_8( bbox + j );
// 
// }

i = (int *) (buf + 36);

n_parts = i[0];

n_points = i[1];

if ( n_parts > max_shp_parts )  {

   cerr << "\n\n  ShpPolylineRecord::set(unsigned char *) -> too many parts ... "
        << " increase parameter \"max_shp_parts\" to at least "
        << n_parts << "\n\n";

   exit ( 1 );

}

if ( n_points > max_shp_points )  {

   cerr << "\n\n  ShpPolylineRecord::set(unsigned char *) -> too many points ... "
        << " increase parameter \"max_shp_points\" to at least "
        << n_points << "\n\n";

   exit ( 1 );

}

   //
   //  read parts array
   //

bytes = n_parts*sizeof(int);

memcpy(parts, buf + 44, bytes);

   //
   //  read points array
   //

offset = 44 + bytes;

bytes = n_points*2*sizeof(double);

memcpy(points, buf + offset, bytes);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ShpPolylineRecord::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);


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

out << prefix << " parts = [ ";

for (j=0; j<n_parts; ++j)  {

   out << parts[j] << ' ';

}

out << "]\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void ShpPolygonRecord::set(unsigned char * buf)

{

// int j;
int * i = (int *) buf;
int bytes, offset;


shape_type = i[0];

if ( shape_type != shape_type_polygon )  {

   cerr << "\n\n  ShpPolygonRecord::set(unsigned char *) -> bad shape type ... "
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

memcpy(bbox, buf + 4, 4*sizeof(double));

// for (j=0; j<4; ++j)  {
// 
//    shuffle_8( bbox + j );
// 
// }

i = (int *) (buf + 36);

n_parts = i[0];

n_points = i[1];

if ( n_parts > max_shp_parts )  {

   cerr << "\n\n  ShpPolygonRecord::set(unsigned char *) -> too many parts ... "
        << " increase parameter \"max_shp_parts\" to at least "
        << n_parts << "\n\n";

   exit ( 1 );

}

if ( n_points > max_shp_points )  {

   cerr << "\n\n  ShpPolygonRecord::set(unsigned char *) -> too many points ... "
        << " increase parameter \"max_shp_points\" to at least "
        << n_points << "\n\n";

   exit ( 1 );

}

   //
   //  read parts array
   //

bytes = n_parts*sizeof(int);

memcpy(parts, buf + 44, bytes);

   //
   //  read points array
   //

offset = 44 + bytes;

bytes = n_points*2*sizeof(double);

memcpy(points, buf + offset, bytes);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ShpPolygonRecord::dump(ostream & out, int depth) const

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

out << prefix << " parts = [ ";

for (j=0; j<n_parts; ++j)  {

   out << parts[j] << ' ';

}

out << "]\n";

out << prefix << " points = [ \n";

for (j=0; j<n_points; ++j)  {

   out << p2 << '(' << points[j].x << ", " << points[j].y << ")\n";

}

out << "]\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


double ShpPolygonRecord::lat(int k) const

{

if ( (k < 0) || (k >= n_points) )  {

   cerr << "\n\n  ShpPolygonRecord::lat(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( points[k].y );

}


////////////////////////////////////////////////////////////////////////


double ShpPolygonRecord::lon(int k) const

{

if ( (k < 0) || (k >= n_points) )  {

   cerr << "\n\n  ShpPolygonRecord::lon(int) const -> range check error\n\n";

   exit ( 1 );

}

double t = points[k].x;

return ( -t );

}


////////////////////////////////////////////////////////////////////////


int ShpPolygonRecord::start_index(int partno) const

{

if ( (partno < 0) || (partno >= n_parts) )  {

   cerr << "\n\n  ShpPolygonRecord::start_index(int) const -> range check error\n\n";

   exit ( 1 );


}

return ( parts[partno] );

}


////////////////////////////////////////////////////////////////////////


int ShpPolygonRecord::stop_index(int partno) const

{

if ( (partno < 0) || (partno >= n_parts) )  {

   cerr << "\n\n  ShpPolygonRecord::stop_index(int) const -> range check error\n\n";

   exit ( 1 );


}

if ( partno == (n_parts - 1) )  return ( n_points - 1 );

return ( parts[partno + 1] - 1 );

}


////////////////////////////////////////////////////////////////////////





