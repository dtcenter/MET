

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////


const char * string_att(const NcFile & Nc, const char * name)

{

NcAtt * att = Nc.get_att(name);

if ( att->type() != ncChar )  {

   cerr << "\n\n   string_att() -> attribute \"" << name << "\" is not a character string!\n\n";

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


