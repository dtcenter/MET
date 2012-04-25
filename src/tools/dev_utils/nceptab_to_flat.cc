

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


////////////////////////////////////////////////////////////////////////


static void process(const char * filename);

static bool parse_line(const char * line, int table_number);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc == 1 )  {

   cerr << "\n\n   usage: " << program_name << " path_to_nceptab_xxx.c [ others ...]\n\n";

   exit ( 1 );

}

int j;

cout << "GRIB1\n";

for (j=1; j<argc; ++j)  {   //  j starts at one, here

   process(argv[j]);

}

   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void process(const char * input_filename)

{

int table_number;
ConcatString line;
ifstream in;
bool found = false;
const char * short_name = get_short_name(input_filename);

   //
   //  grab the table number from the filename
   //

if ( sscanf(short_name, "nceptab_%d.c", &table_number) != 1 )  {

   mlog << Error
        << "\n\n  " << program_name << ": can't get table number from filename \"" << short_name << "\"\n\n";

   exit ( 1 );

}


   //
   //  open the input file
   //

in.open(input_filename);

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

   if ( strstr(line, target_start) != 0 )  { found = true;  break; }

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

   if ( ! parse_line(line, table_number) )  break;

}

   //
   //  done
   //

in.close();

return;

}


////////////////////////////////////////////////////////////////////////


bool parse_line(const char * line, int table_number)

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


