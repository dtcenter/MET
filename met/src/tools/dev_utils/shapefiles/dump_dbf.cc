

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

#include "dbf_file.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static const int buf_size = 65536;

static unsigned char buf[buf_size];


////////////////////////////////////////////////////////////////////////


static void usage();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 2 )  usage();

int fd = -1;
int j, pos;
int n_read, bytes;
ConcatString input_filename = argv[1];
DbfHeader h;
DbfSubRecord sr;


if ( (fd = open(input_filename.contents(), O_RDONLY)) < 0 )  {

   cerr << "\n\n  " << program_name << ": unable to open input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

cout << get_short_name(input_filename) << '\n';

   //
   //  main header
   //

bytes = 32;

if ( (n_read = read(fd, buf, bytes)) != bytes )  {

   cerr << "\n\n  " << program_name << ": trouble reading main file header from input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

h.set_header(buf);

cout << "DbfHeader ...\n";

h.dump(cout, 1);

cout << "\n";

pos = lseek(fd, 0, SEEK_CUR);

cout << "\n  File position = " << comma_string(pos) << "\n\n";

   //
   //  subrecords
   //

h.set_subrecords(fd);

h.dump(cout);

cout << "\n";

pos = lseek(fd, 0, SEEK_CUR);

cout << "\n  File position = " << comma_string(pos) << "\n\n";

   //
   //  records
   //

if ( lseek(fd, h.pos_first_record, SEEK_SET) < 0 )  {

   cerr << "\n\n  " << program_name << ": lseek error\n\n";

   exit ( 1 );

}

pos = lseek(fd, 0, SEEK_CUR);

cout << "\n  File position = " << comma_string(pos) << "\n\n";


for (j=0; j<(h.n_records); ++j)  {

   bytes = h.record_length;

   n_read = read(fd, buf, bytes);

   if ( n_read != bytes )  {

      cerr << "\n\n  " << program_name << ": read error ... n_read = " << n_read << "\n\n";

      exit ( 1 );

   }

   buf[h.record_length] = 0;

   cout << "Record " << j << " ...\n";

   cout << ((char *) buf) << "\n";

}   //  for j


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


