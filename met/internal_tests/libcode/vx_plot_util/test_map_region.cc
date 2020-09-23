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
#include <cmath>

#include "vx_util.h"
#include "map_region.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static int max_points = 0;


////////////////////////////////////////////////////////////////////////


static void usage();

static void process(const char * map_data_filename);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc == 1 )  usage();

int j;

max_points = 0;

for (j=1; j<argc; ++j)  {

   cout << program_name << ": Testing map data file \"" << argv[j] << "\"\n" << flush;

   process(argv[j]);

}

cout << "\n\n  max # of points in one region = " << max_points << "\n\n";

   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " map_data_file_list\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void process(const char * map_data_filename)

{

ifstream in;
MapRegion r;
int count;

met_open(in, map_data_filename);

if ( !in )  {

   cerr << "\n\n   " << program_name << ": unable to open map data file \"" << map_data_filename << "\"\n\n";

   exit ( 1 );

}

count = 0;

while ( in >> r )  {

   ++count;

   if ( r.n_points > max_points )  max_points = r.n_points;

}

cout << "   read " << count << " regions\n" << flush;

   //
   //  done
   //

in.close();

return;

}


////////////////////////////////////////////////////////////////////////


