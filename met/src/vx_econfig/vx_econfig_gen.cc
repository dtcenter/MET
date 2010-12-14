

////////////////////////////////////////////////////////////////////////


static const char * const program_name = "vx_econfig_gen";


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "gen.h"


////////////////////////////////////////////////////////////////////////


static const char * class_name = (const char *) 0;

static const char * file_prefix = (const char *) 0;

static const char * raplib = (const char *) 0;

static int Panic = 1;  //  have generated class panic if not all expected 
                       //  symbols are found in a config file

static int hh = 0;


////////////////////////////////////////////////////////////////////////


static void parse_command_line(int & argc, char ** argv);

static void shift_down(int & argc, char ** argv, int pos, int shift);

static void usage();

static void set_class_name(const char *);

static void set_file_prefix(const char *);

static void set_raplib(const char *);

static void set_nopanic();

static void set_hh();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

parse_command_line(argc, argv);

CodeGenerator codegen;

if(class_name)  codegen.set_class_name(class_name);

if(file_prefix) codegen.set_file_prefix(file_prefix);

if(raplib)      codegen.set_raplib(raplib);

if(hh)          codegen.set_hh();

if(!Panic)      codegen.set_nopanic();


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

        if ( strcmp(argv[j], "-class"   ) == 0 )  { set_class_name  (argv[j + 1]);  shift_down(argc, argv, j, 2); }
   else if ( strcmp(argv[j], "-outfile" ) == 0 )  { set_file_prefix (argv[j + 1]);  shift_down(argc, argv, j, 2); }
   else if ( strcmp(argv[j], "-raplib"  ) == 0 )  { set_raplib      (argv[j + 1]);  shift_down(argc, argv, j, 2); }
   else if ( strcmp(argv[j], "-nopanic" ) == 0 )  { set_nopanic     ();             shift_down(argc, argv, j, 1); }
   else if ( strcmp(argv[j], "-hh"      ) == 0 )  { set_hh          ();             shift_down(argc, argv, j, 1); }
   else {

      cout << "\n\n  unrecognized switch: \"" << argv[j] << "\"\n\n";

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

cerr << "\n\n  usage: " << program_name
     << " -class name"
     << " -outfile name"
     << " [ -nopanic ]"
     << " [ -hh ]"
     << " [ -raplib ]"
     << " config_filename"
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


void set_raplib(const char * text)

{

raplib = text;

return;

}


////////////////////////////////////////////////////////////////////////


void set_nopanic()

{

Panic = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void set_hh()

{

hh = 1;

return;

}


////////////////////////////////////////////////////////////////////////






