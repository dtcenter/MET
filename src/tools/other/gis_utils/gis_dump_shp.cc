

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"
#include "vx_log.h"

#include "shp_file.h"
#include "shp_poly_record.h"
#include "shp_point_record.h"
#include "shapetype_to_string.h"



#include "shp_file.h"
#include "shp_poly_record.h"
#include "shp_point_record.h"
#include "shapetype_to_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //


static bool header_only = false;


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_header_only(const StringArray &);

static void do_poly_dump  (ShpFile &);

static void do_point_dump (ShpFile &);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_header_only, "-h", 0);

cline.parse();

if ( cline.n() != 1 )  usage();


 ConcatString input_filename = (string)cline[0];
ShpFile f;


cout << "file = \"" << get_short_name(input_filename.c_str()) << "\"\n\n";

if ( ! f.open(input_filename.c_str()) )  {

   mlog << Error
        << "\n\n  " << program_name << ": unable to open input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  header
   //

cout << "File Header ... \n";

f.header()->dump(cout, 1);

cout << '\n' << flush;

if ( header_only )   return ( 0 );

   //
   //  records
   //

const ShapeType shape_type = (ShapeType) (f.header()->shape_type);

switch ( shape_type )  {

   case shape_type_polygon:   //  fall through
   case shape_type_polyline:  //  fall through
      do_poly_dump(f);
      break;

   case shape_type_point:
      do_point_dump(f);
      break;

   default:
      mlog << Error
           << "\n\n  " << program_name << ": shape file type \""
           << shapetype_to_string(shape_type) << "\" is not supported\n\n";
      exit ( 1 );
      break;

}   //  switch


   //
   //  done
   //

f.close();

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

mlog << Error 
     << "\n\n  usage:  " << program_name << " [ -h ] shp_filename\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


static void set_header_only(const StringArray &)

{

header_only = true;

return;

}


////////////////////////////////////////////////////////////////////////


void do_poly_dump(ShpFile & f)

{

ShpPolyRecord r;



while ( f >> r )  {

   cout << "Record Header ... \n";

   r.rh.dump(cout, 1);

   cout << "\n";

   cout << "Record Data ... \n";

   r.dump(cout, 1);

   cout << "\n";

}   //  while


cout << "\n\n";




   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void do_point_dump (ShpFile & f)

{

ShpPointRecord r;


while ( f >> r )  {

   cout << "Record Header ... \n";

   r.rh.dump(cout, 1);

   cout << "\n";

   cout << "Record Data ... \n";

   r.dump(cout, 1);

   cout << "\n";

}   //  while


cout << "\n\n";


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


