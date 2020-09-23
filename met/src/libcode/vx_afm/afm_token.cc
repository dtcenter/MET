// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include "afm_token.h"
#include "afmtokentype_to_string.h"
#include "afmkeyword_to_string.h"

#include "vx_log.h"


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

  s = "";

clear();


return;

}


////////////////////////////////////////////////////////////////////////


void AfmToken::clear()

{

if ( s != "" )  { s.clear(); }

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

if ( t.s != "" )  {

   set_string(t.s);

}

return;

}


////////////////////////////////////////////////////////////////////////


void AfmToken::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "line_number = " << line_number << "\n";

out << prefix << "type        = " << afmtokentype_to_string(type)  << "\n";

out << prefix << "keyword     = " << afmkeyword_to_string(keyword) << "\n";

out << prefix << "i           = " << i           << "\n";
out << prefix << "d           = " << d           << "\n";

if ( s != "" )  {

   out << prefix << "s           = \"" << s      << "\"\n";

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


void AfmToken::set_string(const ConcatString text)

{

if ( s != "" )  { s.clear(); }

if ( text == "" )  return;

s = text;

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
      x = 0.0;
      mlog << Error << "\nAfmToken::as_double() const -> bad token type!\n\n";
      dump(cerr);
      exit ( 1 );
      break;

};

return ( x );

}


////////////////////////////////////////////////////////////////////////

