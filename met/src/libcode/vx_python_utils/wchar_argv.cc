

////////////////////////////////////////////////////////////////////////


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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "wchar_argv.h"

#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Wchar_Argv
   //


////////////////////////////////////////////////////////////////////////


Wchar_Argv::Wchar_Argv()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Wchar_Argv::~Wchar_Argv()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Wchar_Argv::Wchar_Argv(const Wchar_Argv & w)

{

mlog << Error
     << "\n\n  Wchar_Argv::Wchar_Argv(const Wchar_Argv &) -> should never be called!n\n";

exit ( 1 );

   ///////////////////////////

init_from_scratch();

// assign(w);


return;

}


////////////////////////////////////////////////////////////////////////


Wchar_Argv & Wchar_Argv::operator=(const Wchar_Argv & w)

{

mlog << Error
     << "\n\n  Wchar_Argv::operator=(const Wchar_Argv &) -> should never be called!n\n";

exit ( 1 );

   ///////////////////////////

if ( this == &w )  return ( * this );

// assign(w);


return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Wchar_Argv::init_from_scratch()

{

W_Buf = 0;

W_Argv = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void Wchar_Argv::clear()

{

if ( W_Argv )  { delete [] W_Argv;  W_Argv = 0; }

if ( W_Buf )  { delete [] W_Buf;  W_Buf = 0; }

Argc = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void Wchar_Argv::set(int _argc, char ** _argv)

{

clear();

int j, k;
int len;
int argv_len;
const char * argv_start = _argv[0];


Argc = _argc;


argv_len = 0;

   //
   //  total length of the argument string ... 
   //

for (j=0; j<_argc; ++j)  {

   argv_len += strlen(_argv[j]);

   ++argv_len;   //  ... including the end-of-string sentinels (ie, nul chars);

}


W_Buf = new wchar_t [argv_len];

for (j=0; j<Argc; ++j)  {

   W_Buf[j] = L'\0';

}


k = 0;

for (j=0; j<Argc; ++j)  {

   len = strlen(_argv[j]);

   if ( mbstowcs(W_Buf + k, _argv[j], len) == (size_t) -1 )  {

      mlog << Error
           << "\n\n  Wchar_Argv::set() -> mbstowcs failed for string \"" << _argv[j] << "\"\n\n";

      exit ( 1 );

   }

   k += (len + 1);

}




W_Argv = new wchar_t * [Argc];

for (j=0; j<Argc; ++j)  {

   k = (int) (_argv[j] - argv_start);

   W_Argv[j] = W_Buf + k;

}



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////





