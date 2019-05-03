

////////////////////////////////////////////////////////////////////////


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

#include "vx_util.h"
#include "vx_log.h"
#include "check_endian.h"

#include "shp_file.h"
#include "shp_point_record.h"
#include "shapetype_to_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct ShpPointRecord
   //


////////////////////////////////////////////////////////////////////////


void ShpPointRecord::set(unsigned char * buf)

{

handle_little_4(buf);
handle_little_4(buf +  4);
handle_little_4(buf + 12);

memcpy(&shape_type, buf, 4);

memcpy(&x, buf +  4, 8);
memcpy(&y, buf + 12, 8);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ShpPointRecord::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "shape_type = " << shapetype_to_string((ShapeType) shape_type) << "\n";

out << prefix << "\n";

out << prefix << "(x, y)     = (" << x << ", " << y << ")\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


bool operator>>(ShpFile & file, ShpPointRecord & record)

{

unsigned char rh_buf[shp_record_header_bytes];
unsigned char xy_buf[shp_point_record_bytes];


   //
   //  read the record header
   //

if ( ! file.read(rh_buf, shp_record_header_bytes) )  {

   if ( file.at_eof() )  return ( false );

   mlog << Error
        << "\n\n  operator>>(ShpFile &, ShpPointRecord &) -> trouble reading record header from shp file \""
        << (file.filename()) << "\"\n\n";

   exit ( 1 );

}

record.rh.set(rh_buf);

   //
   //  read the record
   //

if ( record.rh.content_length_bytes > shp_point_record_bytes )  {

   mlog << Error
        << "\n\n  operator>>(ShpFile &, ShpPointRecord &) -> buffer too small ... increase to at least "
        << (record.rh.content_length_bytes) << "\n\n";

   exit ( 1 );

}

if ( ! file.read(xy_buf, record.rh.content_length_bytes) )  {

   mlog << Error
        << "\n\n  operator>>(ShpFile &, ShpPointRecord &) -> read error in shp file \""
        << (file.filename()) << "\"\n\n";

   exit ( 1 );

}

record.set(xy_buf);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////





