

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "vx_util.h"
#include "gen.h"


////////////////////////////////////////////////////////////////////////


const char * program_name = (const char *) 0;   //  external linkage


////////////////////////////////////////////////////////////////////////


static const char * class_name = (const char *) 0;

static const char * file_prefix = (const char *) 0;

static bool Panic = true;  //  have generated class panic if not all expected
                           //  symbols are found in a config file

static bool allow_multiple_reads = false;

static int hh = 0;


////////////////////////////////////////////////////////////////////////


static void parse_command_line(int & argc, char ** argv);

static void shift_down(int & argc, char ** argv, int pos, int shift);

static void usage();

static void set_class_name(const char *);

static void set_file_prefix(const char *);

static void set_nopanic();

static void set_multiple_reads();

static void set_hh();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

parse_command_line(argc, argv);

CodeGenerator codegen;


codegen.set_class_name(class_name);

codegen.set_file_prefix(file_prefix);


if ( hh )  codegen.set_hh();

if ( !Panic )  codegen.set_nopanic();

codegen.set_multiple_reads(allow_multiple_reads);


codegen.process(argv[1]);




   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void parse_command_line(int & argc, char ** argv)

{

if ( argc == 1 )  { usage();  exit ( 1 ); }

int j, found;

do {

   found = 0;

   for (j=1; j<argc; ++j)  {

      if ( argv[j][0] == '-' )  { found = 1;  break; }

   }

   if ( !found )  break;

        if ( strcmp(argv[j], "-class"                ) == 0 )  { set_class_name     (argv[j + 1]);  shift_down(argc, argv, j, 2); }
   else if ( strcmp(argv[j], "-outfile"              ) == 0 )  { set_file_prefix    (argv[j + 1]);  shift_down(argc, argv, j, 2); }
   else if ( strcmp(argv[j], "-nopanic"              ) == 0 )  { set_nopanic        ();             shift_down(argc, argv, j, 1); }
   else if ( strcmp(argv[j], "-allow_multiple_reads" ) == 0 )  { set_multiple_reads ();             shift_down(argc, argv, j, 1); }
   else if ( strcmp(argv[j], "-hh"                   ) == 0 )  { set_hh             ();             shift_down(argc, argv, j, 1); }
   else {

      mlog << Error << "\n\n  unrecognized switch: \"" << argv[j] << "\"\n\n";

      exit ( 1 );

   }

}  while ( found );



if ( (argc != 2) || !class_name || !file_prefix )  {

   usage();

   exit ( 1 );

}



return;

}


////////////////////////////////////////////////////////////////////////


void shift_down(int & argc, char ** argv, int pos, int shift)

{

int j;

for (j=pos; j<(argc - shift); ++j)  argv[j] = argv[j + shift];

argc -= shift;

return;

}


////////////////////////////////////////////////////////////////////////


void usage()

{

const char * tab = "            ";

mlog << Error << "\n\n  usage: " << program_name << "\n"
     << tab << " [ -allow_multiple_reads ]\n"
     << tab << " [ -nopanic ]\n"
     << tab << " [ -hh ]\n"
     << tab << "     -class name\n"
     << tab << "     -outfile name\n"
     << tab << "     config_filename\n"
     << "\n\n";


return;

}


////////////////////////////////////////////////////////////////////////


void set_class_name(const char * text)

{

class_name = text;

return;

}


////////////////////////////////////////////////////////////////////////


void set_file_prefix(const char * text)

{

file_prefix = text;

return;

}


////////////////////////////////////////////////////////////////////////


void set_nopanic()

{

Panic = false;

return;

}


////////////////////////////////////////////////////////////////////////


void set_multiple_reads()

{

allow_multiple_reads = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_hh()

{

hh = 1;

return;

}


////////////////////////////////////////////////////////////////////////






