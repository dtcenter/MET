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

#include "vx_util/vx_util.h"
#include "vx_math/vx_math.h"
#include "vx_econfig/result.h"
#include "vx_econfig/resulttype_to_string.h"


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

Sval = (char *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Result::clear()

{

Ival = 0;

Dval = 0.0;

if ( Sval )  { delete [] Sval;  Sval = (char *) 0; }

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

} else if ( r.Type == result_double )  {

   Dval = r.Dval;

} else if ( r.Type == result_string )  {

   int n = strlen(r.Sval);

   Sval = new char [1 + n];

   memset(Sval, 0, 1 + n);

   strcpy(Sval, r.Sval);

}


Type = r.Type;


return;

}


////////////////////////////////////////////////////////////////////////


void Result::dump(ostream & out, int depth) const

{

Indent prefix(depth);
char junk[256];


resulttype_to_string(Type, junk);

out << prefix << "Type = " << junk << "\n";

switch ( Type )  {

   case result_int:
      out << prefix << "Ival = " << Ival << "\n";
      break;

   case result_double:
      out << prefix << "Dval = " << Dval << "\n";
      break;

   case result_string:
      out << prefix << "Sval = \"" << Sval << "\"\n";
      break;


   default:
      cerr << "\n\n  Result::dump() const -> don't know how to dump type \""
           << junk << "\"\n\n";
      exit ( 1 );

      break;

}



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


const int n = strlen(text);

Sval = new char [1 + n];

memset(Sval, 0, 1 + n);

strcpy(Sval, text);


Type = result_string;

return;

}


////////////////////////////////////////////////////////////////////////


int Result::ival() const

{

int k;
char junk[256];


switch ( Type )  {

   case result_int:
      k = Ival;
      break;

   case result_double:
      k = nint(Dval);
      break;

   case result_string:
      k = atoi(Sval);
      break;

   default:
      resulttype_to_string(Type, junk);
      cerr << "\n\n  Result::ival() const -> bad type -> " << junk << "\n\n";
      exit ( 1 );
      break;

}   //  switch



return ( k );

}


////////////////////////////////////////////////////////////////////////


double Result::dval() const

{

double x;
char junk[256];


switch ( Type )  {

   case result_int:
      x = (double) Ival;
      break;

   case result_double:
      x = Dval;
      break;

   case result_string:
      x = atof(Sval);
      break;

   default:
      resulttype_to_string(Type, junk);
      cerr << "\n\n  Result::dval() const -> bad type -> " << junk << "\n\n";
      exit ( 1 );
      break;

}   //  switch



return ( x );

}


////////////////////////////////////////////////////////////////////////


const char * Result::sval() const

{

if ( Type != result_string )  {

   char junk[256];

   resulttype_to_string(Type, junk);

   cerr << "\n\n  Result::sval() const -> bad type -> " << junk << "\n\n";

   exit ( 1 );

}


return ( Sval );

}


////////////////////////////////////////////////////////////////////////


Result::operator int () const

{

return ( ival() );

}


////////////////////////////////////////////////////////////////////////


Result::operator double () const

{

return ( dval() );

}


////////////////////////////////////////////////////////////////////////


   //
   //  operator const char * doesn't work, 
   //
   //     because the memory pointed to by the
   //     return value is immediately deallocated
   //     by Result's destructor
   //

/*
Result::operator const char * () const

{

return ( sval() );

}
*/

////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, const Result & r)

{

switch ( r.type() )  {

   case result_int:     out << (r.ival());  break;
   case result_double:  out << (r.dval());  break;

   case result_string:  out << "\"" << (r.sval()) << "\"";  break;


   default:
      out << "Unknown Result";
      break;

}   //  switch




return ( out );

}


////////////////////////////////////////////////////////////////////////







