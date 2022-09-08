////////////////////////////////////////////////////////////////////////


// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


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

#include "shx_file.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static const int buf_size = 65536;

static unsigned char buf[buf_size];


////////////////////////////////////////////////////////////////////////


static void usage();


////////////////////////////////////////////////////////////////////////


int met_main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 2 )  usage();

int fd = -1;
int n_read, bytes;
int count;
ConcatString input_filename = (std::string)argv[1];
ShxFileHeader h;
ShxRecord r;


if ( (fd = met_open(input_filename.c_str(), O_RDONLY)) < 0 )  {

   mlog << Error
        << "\n\n  " << program_name << ": unable to open input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

std::cout << get_short_name(input_filename.c_str()) << '\n';

   //
   //  main header
   //

bytes = 100;

if ( (n_read = read(fd, buf, bytes)) != bytes )  {

   mlog << Error
        << "\n\n  " << program_name << ": trouble reading main file header from input file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

h.set(buf);

std::cout << "ShxFileHeader ...\n";

h.dump(std::cout, 1);

std::cout << "\n";

   //
   //  records
   //

count = 0;

while ( (n_read = read(fd, buf, 8)) == 8 )  {

   r.set(buf);

   std::cout << "Record " << (count++) << " ... \n";

   r.dump(std::cout, 1);

   std::cout << "\n";

}   //  while

std::cout << "\n\n   Read " << count << " records\n\n";

   //
   //  done
   //

close(fd);

return ( 0 );

}


////////////////////////////////////////////////////////////////////////

const std::string get_tool_name() {
   return "gis_dump_shx";
}


////////////////////////////////////////////////////////////////////////


void usage()

{

mlog << Error
     << "\n\n  usage:  " << program_name << " shx_filename\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


