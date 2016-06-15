

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

#include "shp_file.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static const int buf_size = 500000;

static unsigned char buf[buf_size];


////////////////////////////////////////////////////////////////////////


static void usage();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 2 )  usage();

int fd = -1;
int n_read, bytes;
ConcatString input_filename = argv[1];
ShpFileHeader h;
ShpRecordHeader rh;
ShpPolygonRecord gr;


if ( (fd = open(input_filename.contents(), O_RDONLY)) < 0 )  {

   cerr << "\n\n  " << program_name << ": unable to open input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

cout << get_short_name(input_filename) << '\n';

   //
   //  main header
   //

bytes = 100;

if ( (n_read = read(fd, buf, bytes)) != bytes )  {

   cerr << "\n\n  " << program_name << ": trouble reading main file header from input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

h.set(buf);

cout << "ShpFileHeader ...\n";

h.dump(cout, 1);

cout << "\n";

   //
   //  records
   //

while ( (n_read = read(fd, buf, 8)) == 8 )  {

   rh.set(buf);

   cout << "Record Header ... \n";

   rh.dump(cout, 1);

   cout << "\n";

   bytes = rh.content_length_bytes;

   if ( bytes >= buf_size )  {

      cerr << "\n\n  " << program_name << ": buffer too small ... increase to at least "
           << bytes << "\n\n";

      exit ( 1 );

   }

   if ( (n_read = read(fd, buf, bytes)) != bytes )  {

      cerr << "\n\n  " << program_name << ": trouble reading record data ... n_read = " << n_read << "\n\n";

      exit ( 1 );

   }

   gr.set(buf);

   cout << "Record Data ... \n";

   if ( rh.record_number_0 == 1363 )  {

      gr.dump(cout, 1);

   }

   cout << "\n";

}   //  while

cout << "\n\n  n_read = " << n_read << "\n\n";


   //
   //  done
   //

close(fd);  fd = -1;

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n  usage:  " << program_name << " shp_filename\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


