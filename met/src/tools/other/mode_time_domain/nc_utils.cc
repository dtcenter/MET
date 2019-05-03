// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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
#include <cstdio>
#include <cmath>

#include "nc_utils.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


const char * string_att(const NcFile & Nc, const char * name)

{

NcAtt * att = Nc.get_att(name);

if ( att->type() != ncChar )  {

   mlog << Error << "\n\n   string_att() -> attribute \"" << name << "\" is not a character string!\n\n";

   exit ( 1 );

}

const char * c = att->as_string(0);

   //
   //  done
   //

return ( c );

}


////////////////////////////////////////////////////////////////////////


double string_att_as_double (const NcFile & Nc, const char * name)

{

const char * c = string_att(Nc, name);

double value = atof(c);

return ( value );

}


////////////////////////////////////////////////////////////////////////


int string_att_as_int (const NcFile & Nc, const char * name)

{

const char * c = string_att(Nc, name);

int k = atoi(c);

return ( k );

}


////////////////////////////////////////////////////////////////////////


long long string_att_as_ll (const NcFile & Nc, const char * name)

{

const char * c = string_att(Nc, name);

long long k = atoll(c);

return ( k );

}


////////////////////////////////////////////////////////////////////////

   //
   //  example:  20100517_010000
   //

unixtime parse_start_time(const char * text)

{

int k;
int month, day, year, hour, minute, second;
unixtime t;
const int n = strlen(text);

if ( n != 15 )  {

   mlog << Error << "\n\n  parse_start_time() -> bad string ... \"" << text << "\"\n\n";

   exit ( 1 );

}

k = atoi(text);

year  = k/10000;
month = (k%10000)/100;
day   = k%100;

k = atoi(text + 9);

hour   = k/10000;
minute = (k%10000)/100;
second = k%100;

t = mdyhms_to_unix(month, day, year, hour, minute, second);


   //
   //  done
   //

return ( t );

}


////////////////////////////////////////////////////////////////////////


ConcatString start_time_string(const unixtime t)

{

int month, day, year, hour, minute, second;
char junk[256];

unix_to_mdyhms(t, month, day, year, hour, minute, second);

sprintf(junk, "%04d%02d%02d_%02d%02d%02d", year, month, day, hour, minute, second);

return ( ConcatString(junk) );

}


////////////////////////////////////////////////////////////////////////


