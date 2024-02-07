////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

using namespace std;

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <cmath>

#include "main.h"
#include "vx_util.h"
#include "vx_log.h"

#include "dbf_file.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static const size_t buf_size = 65536;

static unsigned char buf[buf_size];


////////////////////////////////////////////////////////////////////////


static void usage();


////////////////////////////////////////////////////////////////////////


int met_main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 2 )  usage();

int fd = -1;
int j;
size_t n_read, bytes;
ConcatString input_filename = (string)argv[1];
DbfHeader h;
DbfSubRecord sr;


if ( (fd = met_open(input_filename.c_str(), O_RDONLY)) < 0 )  {

   mlog << Error
        << "\n\n  " << program_name << ": unable to open input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

cout << get_short_name(input_filename.c_str()) << '\n';

   //
   //  main header
   //

bytes = 32;

if ( (n_read = read(fd, buf, bytes)) != bytes )  {

   mlog << Error
        << "\n\n  " << program_name << ": trouble reading main file header from input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

h.set_header(buf);

cout << "\n";

   //
   //  subrecords
   //

h.set_subrecords(fd);

h.dump(cout);

cout << "\n";

   //
   //  records
   //

if ( lseek(fd, h.pos_first_record, SEEK_SET) < 0 )  {

   mlog << Error
        << "\n\n  " << program_name << ": lseek error\n\n";

   exit ( 1 );

}

for (j=0; j<(h.n_records); ++j)  {

   bytes = h.record_length;

   n_read = read(fd, buf, bytes < buf_size ? bytes : buf_size);

   if ( n_read != bytes )  {

      mlog << Error
           << "\n\n  " << program_name << ": read error ... n_read = " << n_read << "\n\n";

      exit ( 1 );

   }

   if ( h.record_length < buf_size) {
     buf[h.record_length] = 0;
   }
   
   cout << "Record " << j << " ...\n";

   dump_record(cout, 1, buf, h);

}   //  for j


   //
   //  done
   //

close(fd);

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


const string get_tool_name() {
   return "gis_dump_dbf";
}


////////////////////////////////////////////////////////////////////////


void usage()

{

mlog << Error
     << "\n\n  usage:  " << program_name << " dbf_filename\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


