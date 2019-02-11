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

#include "is_number.h"
#include "substring.h"


////////////////////////////////////////////////////////////////////////


static int is_regular_float(const char * text);

static int is_sci_float(const char * text, int exp_pos);


static int is_digit(const char c);

static int all_digits(const char *);

static int is_sign_char(const char c);

static int is_allowed_float_char(const char c);


////////////////////////////////////////////////////////////////////////


static const char allowed_float_chars [] = "+-Ee.0123456789";

static const int n_allowed_float_chars = strlen(allowed_float_chars);


////////////////////////////////////////////////////////////////////////


int is_number(const char * text)

{


if ( !text )  return ( 0 );

if ( text[0] == 0 )  return ( 0 );


if ( is_integer(text) )  return ( 1 );

if ( is_float(text) )    return ( 1 );



return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int is_integer(const char * text)

{

if ( !text )  return ( 0 );

if ( text[0] == 0 )  return ( 0 );



if ( is_sign_char(text[0]) )  {

   if ( all_digits(text + 1) )  return ( 1 );

} else {

   if ( all_digits(text) )  return ( 1 );

}



return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int is_float(const char * text)

{

if ( !text )  return ( 0 );

const int n = strlen(text);

if ( n == 0 )  return ( 0 );


int j;
int pos;
const char * c = (const char *) 0;


   //
   //  first pass - check that all characters
   //     are legitimate
   //

for (j=0; j<n; ++j)  {

   if ( !is_allowed_float_char(text[j]) )  return ( 0 );

}

   //
   //  scientific notation?
   //

c = strchr(text, 'e');
if ( !c )  c = strchr(text, 'E');

if ( c )  {  //  has an exponent

   pos = (int) (c - text);

   return ( is_sci_float(text, pos) );

}

   //
   //  nope
   //

return ( is_regular_float(text) );

}


////////////////////////////////////////////////////////////////////////


int is_regular_float(const char * text)

{

int j, n;
int dp_count;
int digit_count;
const char * t = (const char *) 0;

t = text;

   //
   //  skip the sign character, if there is one
   //

if ( is_sign_char(t[0]) )  ++t;

   //
   //  the rest must be a string of digits
   //  and decimal points with
   //  at least one digit and
   //  at most one decimal point
   //

n = strlen(t);

digit_count = dp_count = 0;

for (j=0; j<n; ++j)  {

   if ( t[j] == '.' )  ++dp_count;
   else {

      if ( is_digit(t[j]) )  ++digit_count;
      else                   return ( 0 );

   }

}

if ( digit_count == 0 )  return ( 0 );

if ( dp_count > 1 )  return ( 0 );



return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int is_sci_float(const char * text, int exp_pos)

{

int n;
char junk[128];


   //
   //  if the "e" is the first or last character, return "no"
   //

n = strlen(text);

if ( (exp_pos == 0) || (exp_pos == (n - 1)) )  return ( 0 );

   //
   //  grab the mantissa
   //

substring(text, junk, 0, exp_pos - 1);

   //
   //  the mantissa must be a regular (non-sci) float
   //  or a valid integer
   //

if ( !is_regular_float(junk) && !is_integer(junk) )  return ( 0 );

   //
   //  grab the exponent
   //

substring(text, junk, exp_pos + 1, n - 1);

   //
   //  the exponent must be a valid integer
   //

if ( !is_integer(junk) )  return ( 0 );




return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int is_digit(const char c)

{

if ( (c >= '0') && (c <= '9') )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int all_digits(const char * text)

{

if ( text[0] == 0 )  return ( 0 );

const char * c = text;

while ( *c )  {

   if ( !(is_digit(*c)) )  return ( 0 );

   ++c;

}


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int is_sign_char(const char c)

{

if ( (c == '+') || (c == '-') )  return ( 1 );

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int is_allowed_float_char(const char c)

{

int j;


for (j=0; j<n_allowed_float_chars; ++j)  {

   if ( allowed_float_chars[j] == c )  return ( 1 );

}



return ( 0 );

}


////////////////////////////////////////////////////////////////////////


