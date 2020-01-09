

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
#include <cmath>

#include "file_linecount.h"
#include "empty_string.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


int file_linecount(const char * path)

{

int count = 0;
ifstream in;
char c;

met_open ( in, path );

if ( !in )  {

   mlog << Error 
        << "\nfile_linecount() -> unable to open input file \"" << path << "\"\n\n";

   exit ( 1 );

}

count = 0;

while ( in.get(c) )  {

   if ( c == '\n' )  ++count;

}


in.close();

return ( count );

}


////////////////////////////////////////////////////////////////////////


