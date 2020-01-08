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
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


static void usage();

static void reformat(const char * input_filename, const char * output_filename);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 3 )  usage();

char *  input_filename = argv[1];
char * output_filename = argv[2];

reformat(input_filename, output_filename);


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " input_filename output_filename\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void reformat(const char * input_filename, const char * output_filename)

{

int j, k, region_number, n_points;
int a, b;
double lat, lon;
double lat_min, lat_max, lon_min, lon_max;
ifstream in;
ofstream out;
char in_line[512];   // the maximum line length in any of the data files is 66
char out_line[512];

   //
   //  open the files
   //

met_open(in, input_filename);

if ( !in )  {

   cerr << "\n\n  " << program_name << ": reformat() -> unable to open input file \"" << input_filename << "\"\n\n";

   exit ( 1 );

}

 met_open(out, output_filename);

if ( !out )  {

   cerr << "\n\n  " << program_name << ": reformat() -> unable to open output file \"" << output_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  loop through the regions
   //

region_number = 0;


memset(in_line, 0, sizeof(in_line));

while ( in.getline(in_line, sizeof(in_line)) )  {

   n_points = 0;

   ++region_number;

      //
      //  first line
      //

   k = sscanf(in_line, "%d%d%d%lf%lf%lf%lf", 
                       &a, &b, &n_points,
                       &lat_max, &lat_min, &lon_min, &lon_max);

   if ( k != 7 )  {

      cerr << "\n\n  " << program_name << ": trouble reading region!\n\n";

      exit ( 1 );

   }

   memset(out_line, 0, sizeof(out_line));

   snprintf(out_line, sizeof(out_line), "%d %d %.3f %.3f %.3f %.3f", 
                     region_number, n_points, 
                     lat_min, lat_max, lon_min, lon_max);

   out << out_line << '\n' << flush;

      //
      //  data points
      //

   for (j=0; j<n_points; ++j)  {

      memset(in_line, 0, sizeof(in_line));

      if ( ! in.getline(in_line, sizeof(in_line)) )  {

         cerr << "\n\n  " << program_name << ": trouble reading data points from region! ... j = " << j << "\n\n";

         exit ( 1 );

      }

      k = sscanf(in_line, "%lf%lf", &lat, &lon);

      if ( k != 2 )  {

         cerr << "\n\n  " << program_name << ": trouble reading data points from line ... \"" << in_line << "\"\n\n";

         exit ( 1 );

      }

      memset(out_line, 0, sizeof(out_line));

      snprintf(out_line, sizeof(out_line), "%7.3f %8.3f", lat, lon);

      out << out_line << '\n' << flush;

   }   //  for j

   memset(in_line, 0, sizeof(in_line));

}   //  while



   //
   //  done
   //

in.close();
out.close();

return;

}


////////////////////////////////////////////////////////////////////////


