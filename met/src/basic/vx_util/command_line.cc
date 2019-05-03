// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "string_fxns.h"
#include "command_line.h"
#include "is_number.h"
#include "util_constants.h"


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class CLOptionInfo
   //


////////////////////////////////////////////////////////////////////////


CLOptionInfo::CLOptionInfo()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CLOptionInfo::~CLOptionInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CLOptionInfo::CLOptionInfo(const CLOptionInfo & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


CLOptionInfo & CLOptionInfo::operator=(const CLOptionInfo & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfo::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfo::clear()

{

option_text.clear();

Nargs = 0;

f = (CLSetFunction) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfo::assign(const CLOptionInfo & i)

{

clear();

option_text = i.option_text;

Nargs = i.Nargs;

f = i.f;

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfo::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "option_text = ";

if ( option_text.length() == 0 )  out << "(nul)\n";
else                              out << '\"' << option_text << "\"\n";

out << prefix << "Nargs       = " << Nargs << "\n";

out << prefix << "f           = " << (void *) f << "\n";

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //   Code for class CLOptionInfoArray
   //


////////////////////////////////////////////////////////////////////////


CLOptionInfoArray::CLOptionInfoArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CLOptionInfoArray::~CLOptionInfoArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CLOptionInfoArray::CLOptionInfoArray(const CLOptionInfoArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


CLOptionInfoArray & CLOptionInfoArray::operator=(const CLOptionInfoArray & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::init_from_scratch()

{

e = (CLOptionInfo *) 0;

AllocInc = 16;   //  default value

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::clear()

{

if ( e )  { delete [] e;  e = (CLOptionInfo *) 0; }



Nelements = 0;

Nalloc = 0;

// AllocInc = 16;   //  don't reset AllocInc


return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::assign(const CLOptionInfoArray & a)

{

clear();

if ( a.n_elements() == 0 )  return;

add(a);

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::extend(int n)

{

if ( n <= Nalloc )  return;

n = AllocInc*( (n + AllocInc - 1)/AllocInc );

int j;
CLOptionInfo * u = (CLOptionInfo *) 0;

u = new CLOptionInfo [n];

if ( !u )  {

   mlog << Error << "\nCLOptionInfoArray::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for(j=0; j<Nelements; ++j)  {

   u[j] = e[j];

}

if ( e )  { delete [] e;  e = (CLOptionInfo *) 0; }

e = u;

u = (CLOptionInfo *) 0;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "AllocInc  = " << AllocInc  << "\n";

int j;

for(j=0; j<Nelements; ++j)  {

   out << prefix << "Element # " << j << " ... \n";

   e[j].dump(out, depth + 1);

}

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::set_alloc_inc(int n)

{

if ( n < 0 )  {

   mlog << Error << "\nCLOptionInfoArray::set_alloc_int(int) -> bad value ... " << n << "\n\n";

   exit ( 1 );

}

if ( n == 0 )  AllocInc = 16;   //  default value
else           AllocInc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::add(const CLOptionInfo & a)

{

extend(Nelements + 1);

e[Nelements++] = a;

return;

}


////////////////////////////////////////////////////////////////////////


void CLOptionInfoArray::add(const CLOptionInfoArray & a)

{

int j;

extend(Nelements + a.n_elements());

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


CLOptionInfo & CLOptionInfoArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   mlog << Error << "\nCLOptionInfoArray::operator[](int) -> range check error ... " << n << "\n\n";

   exit ( 1 );
}

return ( e[n] );

}


////////////////////////////////////////////////////////////////////////


int CLOptionInfoArray::lookup(const char * name) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( strcmp(name, e[j].option_text) == 0 )  return ( j );

}

   //
   //  nope
   //

return ( -1 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CommandLine
   //


////////////////////////////////////////////////////////////////////////


CommandLine::CommandLine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CommandLine::~CommandLine()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CommandLine::CommandLine(const CommandLine & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


CommandLine & CommandLine::operator=(const CommandLine & c)

{

if ( this == &c )  return ( * this );

assign(c);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CommandLine::init_from_scratch()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void CommandLine::clear()

{

args.clear();

options.clear();

ProgramName.clear();

Usage = (UsageFunction) 0;

AllowNumbers = false;

AllowUnrecognizedSwitches = false;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::assign(const CommandLine & c)

{

clear();

args = c.args;

options = c.options;

ProgramName = c.ProgramName;

Usage = c.Usage;

AllowNumbers = c.AllowNumbers;

return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Program Name = ";

if ( ProgramName.length() > 0 )  out << '\"' << ProgramName << "\"\n";
else                             out << "(nul)\n";

out << prefix << "\n";

out << prefix << "Args ...\n";

args.dump(out, depth + 1);

out << prefix << "\n";

out << prefix << "Options ...\n";

options.dump(out, depth + 1);

out << prefix << "\n";

out << prefix << "AllowNumbers              = " << ( AllowNumbers ? "true" : "false" ) << "\n";;
out << prefix << "AllowUnrecognizedSwitches = " << ( AllowUnrecognizedSwitches ? "true" : "false" ) << "\n";;

return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::set(int argc, char ** argv)

{

clear();

int j;

ProgramName = get_short_name(argv[0]);

for (j=1; j<argc; ++j)  {   //  j starts at one here, not zero

   args.add(argv[j]);

}


return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::shift_down(int pos, int k)

{

   //
   //  any necessary error-checking is done by
   //
   //    StringArray::shift_down()
   //

args.shift_down(pos, k);

return;

}


////////////////////////////////////////////////////////////////////////


const char * CommandLine::operator[](int k) const

{

   //
   //  any necessary error-checking is done by
   //
   //    StringArray::operator[](int)
   //

const char * c = (const char *) 0;

c = args[k];

return ( c );

}


////////////////////////////////////////////////////////////////////////


bool CommandLine::has_option(const char *text) const

{

bool status;

status = ( args.has(text) == 1 );

return ( status );

}


////////////////////////////////////////////////////////////////////////


int CommandLine::next_option(int & option_index) const

{

int j, N;
const char * name = (const char *) 0;


N = args.n_elements();

for (j=0; j<N; ++j)  {

   name = args[j];

   if ( is_switch(name) )  {

      if ( (strcmp(name, "-help") == 0) || (strcmp(name, "--help") == 0) )  do_help();   //  doesn't return

      if ( (strcmp(name, "-version") == 0) || (strcmp(name, "--version") == 0) )  show_version();   //  doesn't return

      option_index = options.lookup(args[j]);

      if ( option_index >= 0 )  return ( j );

        //
        //  option not recognized
        //

      if ( AllowUnrecognizedSwitches )  continue;
      else {

         mlog << Error << "\nCommandLine::next_option() -> unrecognized command-line switch \"" << args[j] << "\"\n\n";

         exit ( 1 );

      }

      return ( j );

   }

}   //  for j

   //
   //  nope
   //

return ( -1 );

}


////////////////////////////////////////////////////////////////////////


void CommandLine::add(CLSetFunction func, const char * text, int n_args)

{

CLOptionInfo info;

info.option_text = text;

info.Nargs = n_args;

info.f = func;

options.add(info);


return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::do_help() const

{

if ( Usage )  Usage();
else {

   mlog << Error << "\n" << ProgramName << ": no usage message available ... sorry.\n\n";

}


exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::show_version() const

{

cout << "\n\n  " << met_version << "\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


int CommandLine::length(int k) const

{

return ( args.length(k) );

}


////////////////////////////////////////////////////////////////////////


void CommandLine::parse()

{

int j, index;
int N, M;
StringArray a;
const char * switch_name = (const char *) 0;


while ( (j = next_option(index)) >= 0 )  {

   switch_name = args[j];

   if ( index < 0 )  {

      mlog << Error << "\n" << ProgramName << ": unrecognized command-line switch: \"" << switch_name << "\"\n\n";

      if ( Usage )  Usage();

      exit ( 1 );

   }

      //
      //  set up the arguments
      //

   N = options[index].Nargs;

   if ( N < 0 )  {

      M = get_unlimited_args (a, j + 1);

   } else {

      M = N;

      get_n_args(a, N, switch_name, j + 1);

   }

   args.shift_down(j, M + 1);

      //
      //  call the function
      //

   options[index].f(a);

}   //  while

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void CommandLine::get_n_args(StringArray & a, const int Nargs, const char * switch_name, const int pos)

{

a.clear();

if ( Nargs == 0 )  return;

int j, k;
const char * c = (const char *) 0;


for (j=0; j<Nargs; ++j)  {

   k = j + pos;

   if ( k >= args.n_elements() )  {

      mlog << Error << "\nCommandLine::get_n_args() -> too few arguments to command-line switch \"" << switch_name << "\"\n\n";

      exit ( 1 );

   }

   c = args[k];

   if ( is_switch(c) )  {

      mlog << Error << "\nCommandLine::get_n_args() -> too few arguments to command-line switch \"" << switch_name << "\"\n\n";

      exit ( 1 );

   }

   a.add(c);

}   //  for j

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int CommandLine::get_unlimited_args(StringArray & a, const int pos)

{

int k, count;
const char * c = (const char *) 0;


a.clear();

count = 0;

while ( 1 )  {

   k = pos + count;

   if ( k >= args.n_elements() )  break;

   c = args[k];

   if ( is_switch(c) )  break;

   a.add(c);

   ++count;

}   //  while


   //
   //  done
   //

return ( count );

}


////////////////////////////////////////////////////////////////////////


bool CommandLine::is_switch(const char * text) const

{

if ( !text || !(*text) )  {

   mlog << Error << "\nCommandLine::is_switch(const char *) const -> empty string!\n\n";

   exit ( 1 );

}

if ( text[0] != '-' )  return ( false );

if ( AllowNumbers && is_number(text) )  return ( false );

return ( true );

}


////////////////////////////////////////////////////////////////////////





