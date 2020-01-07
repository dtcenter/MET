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
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_log.h"
#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


static void parse_line(const char * line);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

  ConcatString program_name = (string)get_short_name(argv[0]);

if ( argc != 2 )  {

   cerr << "\n\n   usage: " << program_name << " path_to_gribtab.dat\n\n";

   exit ( 1 );

}

const char * const input_filename = argv[1];

char line[512];
ifstream in;

met_open(in, input_filename);

if ( ! in )  {

   cerr << "\n\n  " << program_name << ": unable to open input file \"" << input_filename << "\"\n\n";

   exit ( 1 );

}

cout << "GRIB2\n";

while ( in.getline(line, sizeof(line)) )  {

   parse_line(line);

}


in.close();

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void parse_line(const char * line)

{

int j;
int i[6];
const char i_delim [] = "{} ,\"";
const char s_delim [] = ",\"";
const char * c = (const char *) 0;
char line2[512];
char * s = line2;

strncpy(line2, line, sizeof(line2));

   //
   //  get first 6 integers
   //

for (j=0; j<6; ++j)  {

   c = strtok(s, i_delim);

   i[j] = atoi(c);

   s = (char *) 0;

}   //  while

   //
   //  write first and last two numbers 
   //

cout << i[0] << ' ' << i[4] << ' ' << i[5] << ' ';

   //
   //  get remaining strings
   //

for (j=0; j<3; ++j)  {

   c = strtok(s, s_delim);
   c = strtok(s, s_delim);

   cout << ' ' << '\"' << c << '\"';

   s = (char *) 0;

}   //  while


   //
   //  done
   //

cout << '\n' << flush;

return;

}


////////////////////////////////////////////////////////////////////////


