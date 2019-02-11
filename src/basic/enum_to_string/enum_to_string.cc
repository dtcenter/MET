// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


static const int debug = 0;


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cstdio>
#include <cmath>

#include "info.h"
#include "code.h"
#include "scope.h"

#include "vx_log.h"
#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


   //
   //  declarations with external linkage
   //


extern int yyparse();

// extern FILE * yyin;
FILE * yyin = 0;

// extern char * yytext;
char * yytext = 0;

extern int yydebug;

extern "C" int yywrap();

extern EnumInfo einfo;

extern const char * header_filename;

extern int yydebug;

extern ScopeStack ss;

extern ScopeStackElement sse;

extern unixtime generation_gmt;


   //
   //  definitions with external linkage
   //


int LineNumber             = 1;

int column                 = 1;

bool do_prefix             = false;

bool do_concat_string      = false;

const char * header_suffix = ".h";

bool do_angle_brackets     = false;

bool do_array              = false;

bool do_reverse            = false;

bool verbose               = true;

unixtime generation_gmt    = (unixtime) 0;

const char * program_name  = (const char *) 0;


////////////////////////////////////////////////////////////////////////


static void parse_command_line(int &argc, char **argv);

static void shift_down(int &argc, char **argv, int pos, int shift);

static void set_prefix();

static void set_suffix(const char *);

static void set_angle_brackets();

static void set_array();

static void set_reverse();

static void set_concat_string();

static void set_quiet();

static void usage();


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv[])

{

generation_gmt = time(0);

int j = strlen(argv[0]) - 1;

while ( (j >= 0) && (argv[0][j] != '/') )  --j;

++j;

program_name = argv[0] + j;

parse_command_line(argc, argv);


header_filename = argv[1];




/*
if ( dup2(1, 2) < 0 )  {

   cerr << "\n\n  dup2 failed\n\n";

   exit ( 1 );

}
*/


yydebug = debug;

if ( (yyin = met_fopen(header_filename, "r")) == NULL )  {

   cerr << "\n\n  unable to open input file \"" << header_filename << "\"\n\n";

   exit ( 1 );

}


// cout << "\n";

int parse_status;

parse_status = yyparse();

if ( parse_status != 0 )  {

   cerr << "\n\n  parse_status is nonzero! ... = " << parse_status << "\n\n";

   exit ( 1 );

}


// cout << "\n\n";


return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void parse_command_line(int &argc, char **argv)

{

if ( argc == 1 )  { usage();  exit ( 1 ); }

int j, found;

do {

   found = 0;

   for (j=1; j<argc; ++j)  {

      if ( argv[j][0] == '-' )  { found = 1;  break; }

   }

   if ( !found )  break;

        if ( strcmp(argv[j], "-prefix"          ) == 0 )  { set_prefix         ();       shift_down(argc, argv, j, 1); }
   else if ( strcmp(argv[j], "-hh"              ) == 0 )  { set_suffix         (".hh");  shift_down(argc, argv, j, 1); }
   else if ( strcmp(argv[j], "-no_suffix"       ) == 0 )  { set_suffix         (0);      shift_down(argc, argv, j, 1); }
   else if ( strcmp(argv[j], "-angle_brackets"  ) == 0 )  { set_angle_brackets ();       shift_down(argc, argv, j, 1); }
   else if ( strcmp(argv[j], "-array"           ) == 0 )  { set_array          ();       shift_down(argc, argv, j, 1); }
   else if ( strcmp(argv[j], "-reverse"         ) == 0 )  { set_reverse        ();       shift_down(argc, argv, j, 1); }
   else if ( strcmp(argv[j], "-concat_string"   ) == 0 )  { set_concat_string  ();       shift_down(argc, argv, j, 1); }
   else if ( strcmp(argv[j], "-quiet"           ) == 0 )  { set_quiet          ();       shift_down(argc, argv, j, 1); }
   else {

      cout << "\n\n  unrecognized switch: \"" << argv[j] << "\"\n\n";

      exit ( 1 );

   }

}  while ( found );

if ( argc != 2 )  { usage();  exit ( 1 ); }

return;

}


////////////////////////////////////////////////////////////////////////


void shift_down(int &argc, char **argv, int pos, int shift)

{

int j;

for (j=pos; j<(argc - shift); ++j)  argv[j] = argv[j + shift];

argc -= shift;

return;

}


////////////////////////////////////////////////////////////////////////


void set_prefix()

{

do_prefix = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_suffix(const char * s)

{

header_suffix = s;

return;

}


////////////////////////////////////////////////////////////////////////


void set_angle_brackets()

{

do_angle_brackets = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_array()

{

do_array = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_reverse()

{

do_reverse = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_concat_string()

{

do_concat_string = true;

return;

}


////////////////////////////////////////////////////////////////////////


void set_quiet()

{

verbose = false;

return;

}


////////////////////////////////////////////////////////////////////////


void usage()

{

const char tab [] = "             ";


cerr << "\n\n"
     << "   usage:  " << program_name << "\n\n"

     << tab << "[ -prefix ]\n"
     << tab << "[ -hh ]\n"
     << tab << "[ -no_suffix ]\n"
     << tab << "[ -angle_brackets ]\n"
     << tab << "[ -array ]\n"
     << tab << "[ -reverse ]\n"
     << tab << "[ -concat_string ]\n"
     << tab << "[ -quiet ]\n"

     << "\n"
     << tab << "     header_file"
     << "\n\n";


return;

}


////////////////////////////////////////////////////////////////////////


void yyerror(const char * s)

{

int c;

c = (int) (column - strlen(yytext));

cout << "\n\n  " << program_name << " -> "
     << "syntax error in file \"" << header_filename << "\" at line " << LineNumber 
     << ", column " << c
     << " ... text = \"" << yytext << "\"\n\n";

if ( debug )  {

   cout << "sse.........\n";
   cout << sse << "\n\n";

   cout << "ScopeStack.........\n";
   cout << ss << "\n\n";

}

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


int yywrap()

{

return ( 1 );

}


////////////////////////////////////////////////////////////////////////



