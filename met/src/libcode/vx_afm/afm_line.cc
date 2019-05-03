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
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "afm_line.h"
#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


static const int afm_line_size = 256;

static const char delim [] = " ;\015\032";


////////////////////////////////////////////////////////////////////////


static int is_boolean(const char *, int &);

static int is_keeper(const char);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AfmLine
   //


////////////////////////////////////////////////////////////////////////


AfmLine::AfmLine()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AfmLine::~AfmLine()

{

if ( Line )  { delete [] Line;  Line = (char *) 0; }

strtok_pointer = (char *) 0;
tok_pointer = (char *) 0;

}


////////////////////////////////////////////////////////////////////////


AfmLine::AfmLine(const AfmLine & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


AfmLine & AfmLine::operator=(const AfmLine & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AfmLine::init_from_scratch()

{

Line = new char [afm_line_size];

clear();


return;

}


////////////////////////////////////////////////////////////////////////


void AfmLine::clear()

{

memset(Line, 0, afm_line_size);

LineLength = 0;

strtok_pointer = (char *) 0;
tok_pointer = (char *) 0;

line_number = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void AfmLine::assign(const AfmLine & a)

{

memcpy(Line, a.Line, afm_line_size);

strtok_pointer = a.strtok_pointer;

tok_pointer = a.tok_pointer;

LineLength  = a.LineLength;

line_number = a.line_number;



return;

}


////////////////////////////////////////////////////////////////////////


AfmToken AfmLine::nexttoken()

{

int k;
AfmToken tok;
AfmKeyword key;



tok_pointer = strtok(strtok_pointer, delim);

strtok_pointer = (char *) 0;

if ( !tok_pointer )  {

   tok.type = afm_token_endofline;

   return ( tok );

}


tok.column = 1 + ((int) (tok_pointer - Line));

tok.line_number = line_number;

tok.set_string(tok_pointer);



if ( is_integer(tok_pointer) )  {

   tok.i = atoi(tok_pointer);

   tok.type = afm_token_integer;

} else if ( is_float(tok_pointer) )  {

   tok.d = atof(tok_pointer);

   tok.type = afm_token_number;

} else if ( is_afm_keyword(tok_pointer, key) )  {

   tok.type = afm_token_keyword;

   tok.keyword = key;

} else if ( is_boolean(tok_pointer, k) )  {

   tok.type = afm_token_boolean;

   tok.i = k;

} else {   // must be a name

   tok.type = afm_token_name;

   // mlog << Error << "\nAfmLine::nexttoken() -> bad token ... \""
   //      << tok_pointer << "\"\n\n";
   //
   // exit ( 1 );

}






return ( tok );

}


////////////////////////////////////////////////////////////////////////


AfmToken AfmLine::rest_as_string()

{

AfmToken tok;


tok_pointer += 1 + strlen(tok_pointer);

int j = strlen(tok_pointer) - 1;

while ( (j > 0) && !is_keeper(tok_pointer[j]) )  {

   tok_pointer[j] = (char) 0;

}

tok.set_string(tok_pointer);



return ( tok );

}


////////////////////////////////////////////////////////////////////////


int AfmLine::is_ok() const

{

int j;


for (j=0; j<LineLength; ++j)  {

   if ( is_keeper(Line[j]) )  return ( 1 );

}




return ( 0 );

}


////////////////////////////////////////////////////////////////////////



   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


int is_boolean(const char * text, int & truth_value)

{

if ( strcmp(text, "true") == 0 )  {

   truth_value = 1;

   return ( 1 );

}

if ( strcmp(text, "false") == 0 )  {

   truth_value = 0;

   return ( 1 );

}





return ( 0 );

}

////////////////////////////////////////////////////////////////////////


extern istream & operator>>(istream & in, AfmLine & L)

{

L.clear();

L.strtok_pointer = L.Line;

while ( 1 )  {

   in.getline(L.Line, afm_line_size);

   L.LineLength = strlen(L.Line);

   if ( !in )  {

      L.clear();

      return ( in );

   }

   if ( L.is_ok() )  return ( in );

}   //  while


}


////////////////////////////////////////////////////////////////////////


int is_keeper(const char c)

{

if ( c == 13 )  return ( 0 );   //  control_m

if ( c == 26 )  return ( 0 );   //  control_z

if ( c == 10 )  return ( 0 );   //  newline

if ( c == 32 )  return ( 0 );   //  space



return ( 1 );

}


////////////////////////////////////////////////////////////////////////





