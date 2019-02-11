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
#include <cmath>

#include "vx_log.h"
#include "pxm_base.h"
#include "pxm_utils.h"


////////////////////////////////////////////////////////////////////////


int parse_number(istream & in)

{

int j;
char c;

skip_whitespace(in);

j = 0;

while ( isdigit(c = in.get()) )  j = 10*j + (c - '0');

return ( j );

}


////////////////////////////////////////////////////////////////////////


void skip_whitespace(istream & in)

{

int c;

while ( 1 )  {

   c = in.peek();

   if ( !isspace(c) )  break;

   c = in.get();   //  consume the character

}

return;

}


////////////////////////////////////////////////////////////////////////


void get_comment(istream & in, char * junk)

{

int k;
char c;

in.get(c);   //  toss leading '#'

k = 0;

while ( 1 )  {

   in.get(c);

   if ( c == '\n' )  break;

   if ( k >= (max_comment_length - 2) )  {

      mlog << Error << "\nvoid get_comment(ifstream &, char *) -> comment too long!\n\n";

      exit ( 1 );

   }

   junk[k++] = c;

}

junk[k] = (char) 0;

return;

}


////////////////////////////////////////////////////////////////////////




