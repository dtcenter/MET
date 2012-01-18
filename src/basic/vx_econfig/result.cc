

/////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "vx_math.h"
#include "result.h"
#include "resulttype_to_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Result
   //


////////////////////////////////////////////////////////////////////////


Result::Result()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Result::~Result()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Result::Result(const Result & r)

{

init_from_scratch();

assign(r);

}


////////////////////////////////////////////////////////////////////////


Result & Result::operator=(const Result & r)

{

if ( this == &r )  return ( * this );

assign(r);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


Result::Result(int k)

{

init_from_scratch();

set_int(k);

}


////////////////////////////////////////////////////////////////////////


Result::Result(bool tf)

{

init_from_scratch();

set_boolean(tf);

}


////////////////////////////////////////////////////////////////////////


Result::Result(double x)

{

init_from_scratch();

set_double(x);

}


////////////////////////////////////////////////////////////////////////


Result::Result(const char * text)

{

init_from_scratch();

set_string(text);

}


////////////////////////////////////////////////////////////////////////


void Result::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Result::clear()

{

Ival = 0;

Dval = 0.0;

Sval.clear();


Type = no_result_type;


return;

}


////////////////////////////////////////////////////////////////////////


void Result::assign(const Result & r)

{

clear();

if ( r.Type == no_result_type )  return;


if ( r.Type == result_int )  {

   Ival = r.Ival;

} else if ( r.Type == result_boolean )  {

   Ival = r.Ival;

} else if ( r.Type == result_double )  {

   Dval = r.Dval;

} else if ( r.Type == result_string )  {

   Sval = r.Sval;

}


Type = r.Type;


return;

}


////////////////////////////////////////////////////////////////////////


void Result::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "Type = " << resulttype_to_string(Type) << "\n";

switch ( Type )  {

   case result_int:
      out << prefix << "Ival = " << Ival << "\n";
      break;

   case result_boolean:
      out << prefix << "Ival = " << ( Ival ? "true" : "false" ) << "\n";
      break;

   case result_double:
      out << prefix << "Dval = " << Dval << "\n";
      break;

   case result_string:
      out << prefix << "Sval = \"" << Sval << "\"\n";
      break;

   case no_result_type:
      break;


   default:
      mlog << Error << "\n  Result::dump() const -> "
           << "don't know how to dump type \""
           << resulttype_to_string(Type) << "\"\n\n";
      exit ( 1 );

      break;

}   //  switch



   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void Result::set_int(int k)

{

clear();

Type = result_int;

Ival = k;

return;

}


////////////////////////////////////////////////////////////////////////


void Result::set_boolean(bool tf)

{

clear();

Type = result_boolean;

Ival = ( tf ? 1 : 0 );

return;

}


////////////////////////////////////////////////////////////////////////


void Result::set_double(double x)

{

clear();

Type = result_double;

Dval = x;

return;

}


////////////////////////////////////////////////////////////////////////


void Result::set_string(const char * text)

{

clear();

Type = result_string;

Sval = text;

return;

}


////////////////////////////////////////////////////////////////////////


int Result::ival() const

{

int k;


switch ( Type )  {

   case result_int:
      k = Ival;
      break;

   case result_boolean:
      k = ( Ival ? 1 : 0 );
      break;

   case result_double:
      k = nint(Dval);
      break;

   case result_string:
      k = atoi(Sval);
      break;

   default:
      mlog << Error << "\n  Result::ival() const -> "
           << "bad type ... \"" << resulttype_to_string(Type) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch



return ( k );

}


////////////////////////////////////////////////////////////////////////


bool Result::bval() const

{

int k;


switch ( Type )  {

   case result_int:
   case result_boolean:
      k = ( Ival ? 1 : 0 );
      break;

   case result_double:
      k = nint(Dval);
      break;

   case result_string:
      k = atoi(Sval);
      break;

   default:   
      mlog << Error << "\n  Result::ival() const -> "
           << "bad type ... \"" << resulttype_to_string(Type) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch


return ( ( k ? true : false ) );

}


////////////////////////////////////////////////////////////////////////


double Result::dval() const

{

double x;


switch ( Type )  {

   case result_int:
   case result_boolean:
      x = (double) Ival;
      break;

   case result_double:
      x = Dval;
      break;

   case result_string:
      x = atof(Sval);
      break;

   default:
      mlog << Error << "\n  Result::dval() const -> "
           << "bad type ... \"" << resulttype_to_string(Type) << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch



return ( x );

}


////////////////////////////////////////////////////////////////////////


const char * Result::sval() const

{

if ( Type != result_string )  {

   mlog << Error << "\n  Result::sval() const -> "
        << "bad type ... \"" << resulttype_to_string(Type) << "\"\n\n";

   exit ( 1 );

}


return ( (const char *) Sval );

}


////////////////////////////////////////////////////////////////////////


Result::operator int () const

{

return ( ival() );

}


////////////////////////////////////////////////////////////////////////


Result::operator bool () const

{

return ( bval() );

}


////////////////////////////////////////////////////////////////////////


Result::operator double () const

{

return ( dval() );

}


////////////////////////////////////////////////////////////////////////


Result::operator const char * () const

{

return ( sval() );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, const Result & r)

{

switch ( r.type() )  {

   case result_int:      out << (r.ival());  break;
   case result_boolean:  out << (r.bval());  break;
   case result_double:   out << (r.dval());  break;

   case result_string:   out << "\"" << (r.sval()) << "\"";  break;


   default:
      out << "Unknown Result";
      break;

}   //  switch




return ( out );

}


////////////////////////////////////////////////////////////////////////







