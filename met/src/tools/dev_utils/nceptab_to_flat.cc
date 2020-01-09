// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


   //
   //  Takes a bunch of C source files from the wgrib code 
   //
   //    (example: nceptab_128.c) and writes
   //
   //    a flat file to stdout
   //


////////////////////////////////////////////////////////////////////////


static const char target_start [] = "const struct";


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


ConcatString program_name;

CommandLine cline;

static bool table_number_set = false;

static int table_number = 0;


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_table_number(const StringArray &);

static void process(const char * filename);

static bool parse_line(const char * line);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_table_number, "-table_number", 1);

cline.parse();

if ( cline.n() == 0 )  usage();

int j;

cout << "GRIB1\n";

for (j=0; j<(cline.n()); ++j)  {   //  j starts at one, here

   process(cline[j].c_str());

}

   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage: " << program_name << " nceptab_source_file.c [ more such files ...]\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_table_number(const StringArray & a)

{

table_number = atoi(a[0].c_str());

table_number_set = true;

return;

}


////////////////////////////////////////////////////////////////////////


void process(const char * input_filename)

{

ConcatString line;
ifstream in;
bool found = false;
const char * short_name = get_short_name(input_filename);

   //
   //  grab the table number from the filename
   //

if ( ! table_number_set )  {

   if ( sscanf(short_name, "nceptab_%d.c", &table_number) != 1 )  {

      mlog << Error
           << "\n\n  " << program_name << ": can't get table number from filename \"" << short_name << "\"\n\n";

      exit ( 1 );

   }

}


   //
   //  open the input file
   //

met_open(in, input_filename);

if ( ! in )  {

   mlog << Error
        << "\n\n  " << program_name << ": unable to open input file \"" << input_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  look for a line containing target_start
   //

found = false;

while ( line.read_line(in) )  {

   if ( strstr(line.c_str(), target_start) != 0 )  { found = true;  break; }

}

if ( !found )  {

   mlog << Error
        << program_name << ": target string \"" << target_start << "\" not found in file \""
        << input_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  parse the file
   //


while ( 1 )  {

   line.read_line(in);

   if ( ! parse_line(line.c_str()) )  break;

}

   //
   //  done
   //

in.close();

return;

}


////////////////////////////////////////////////////////////////////////


bool parse_line(const char * line)

{

int n, k;
char line2[1024];
char * s = line2;
char * c = (char *) 0;

strncpy(line2, line, sizeof(line2));

   //
   // check to make sure it's got a double quote in it
   //

if ( ! strchr(line, '\"') )  return ( false );

   //
   //  index and table number
   //

c = strtok(s, " /*");

s = (char *) 0;

if ( !c )  return ( false );

n = atoi(c);

cout << n << ' ' << table_number << ' ';

   //
   //  first string
   //

c = strtok(s, " /*{\"");

if ( !c )  return ( false );

cout << '\"' << c << "\" ";

   //
   //  second string (is this football?)
   //

c = strtok(s, ",\"");
c = strtok(s, ",\"[");

k = strlen(c) - 1;

if ( c[k] == ' ' )  c[k] = (char) 0;

if ( !c )  return ( false );

cout << '\"' << c << "\" ";

   //
   //  units
   //

if ( strcmp(c, "undefined") == 0 )  {

   cout << "\"\" ";

} else {

   c = strtok(s, "]\"");

   if ( !c )  return ( false );

   if ( c[0] == '}' ) cout << "\"\" ";
   else               cout << '\"' << c << "\" ";

}

   //
   //  done
   //

cout << '\n' << flush;

return ( true );

}


////////////////////////////////////////////////////////////////////////


