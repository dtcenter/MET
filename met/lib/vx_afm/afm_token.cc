// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



/////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_afm/afm_token.h"
#include "vx_afm/afmtokentype_to_string.h"
#include "vx_afm/afmkeyword_to_string.h"

#include "vx_util/vx_util.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AfmToken
   //


////////////////////////////////////////////////////////////////////////


AfmToken::AfmToken()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AfmToken::~AfmToken()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AfmToken::AfmToken(const AfmToken & t)

{

init_from_scratch();

assign(t);

}


////////////////////////////////////////////////////////////////////////


AfmToken & AfmToken::operator=(const AfmToken & t)

{

if ( this == &t )  return ( * this );

assign(t);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AfmToken::init_from_scratch()

{

s = (char *) 0;

clear();


return;

}


////////////////////////////////////////////////////////////////////////


void AfmToken::clear()

{

if ( s )  { delete [] s;  s = (char *) 0; }

line_number = column = -1;

type = no_afm_token_type;

keyword = no_afm_keyword;

i = 0;

d = 0.0;


return;

}


////////////////////////////////////////////////////////////////////////


void AfmToken::assign(const AfmToken & t)

{

clear();

line_number = t.line_number;

column = t.column;

type = t.type;

keyword = t.keyword;

i = t.i;

d = t.d;

if ( t.s )  {

   set_string(t.s);

}



return;

}


////////////////////////////////////////////////////////////////////////


void AfmToken::dump(ostream & out, int depth) const

{

Indent prefix(depth);
char junk[256];

out << prefix << "line_number = " << line_number << "\n";
out << prefix << "column      = " << column      << "\n";

afmtokentype_to_string(type, junk);

out << prefix << "type        = " << junk        << "\n";

afmkeyword_to_string(keyword, junk);

out << prefix << "keyword     = " << junk        << "\n";

out << prefix << "i           = " << i           << "\n";
out << prefix << "d           = " << d           << "\n";

if ( s )  {

   out << prefix << "s           = \"" << s           << "\"\n";

} else {

   out << prefix << "s           = (nul)\n";

}

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void AfmToken::set_string(const char * text)

{

if ( s )  { delete [] s;  s = (char *) 0; }

if ( !text )  return;

int n = strlen(text);

s = new char [1 + n];

strcpy(s, text);

return;

}


////////////////////////////////////////////////////////////////////////


double AfmToken::as_double() const

{

double x;


switch ( type )  {


   case afm_token_integer:  x = (double) i;   break;
   case afm_token_number:   x = d;            break;


   default:
      cerr << "\n\n  AfmToken::as_double() const -> bad token type!\n\n";
      dump(cerr);
      exit ( 1 );
      break;

};


return ( x );

}


////////////////////////////////////////////////////////////////////////






