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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "string_array.h"
#include "logger.h"
#include "indent.h"


////////////////////////////////////////////////////////////////////////


static const int stringarray_alloc_inc = 50;


////////////////////////////////////////////////////////////////////////


static int lex_comp (const void *, const void *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class StringArray
   //


////////////////////////////////////////////////////////////////////////


StringArray::StringArray()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


StringArray::~StringArray()

{

clear();

}


////////////////////////////////////////////////////////////////////////


StringArray::StringArray(const StringArray & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


StringArray & StringArray::operator=(const StringArray & a)

{

if ( this == &a )  return ( * this );

assign(a);


return ( * this );

}


////////////////////////////////////////////////////////////////////////


void StringArray::init_from_scratch()

{

s = (char **) 0;

IgnoreCase = 0;

clear();




return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::clear()

{

if ( s )  {

   int j;

   for (j=0; j<Nalloc; ++j)  {

      if ( s[j] )  { delete [] s[j];  s[j] = (char *) 0; }

   }

   delete [] s;  s = (char **) 0;

}   //  if s


Nelements  = Nalloc = 0;

MaxLength  = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::assign(const StringArray & a)

{

clear();

extend(a.Nelements);

int j;

for (j=0; j<(a.Nelements); ++j)  {

   if ( a.s[j] )  add(a.s[j]);

}

Nelements  = a.Nelements;

MaxLength  = a.MaxLength;

IgnoreCase = a.IgnoreCase;

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);
Indent prefix2(depth + 1);

out << prefix << "Nelements  = " << Nelements  << "\n";
out << prefix << "Nalloc     = " << Nalloc     << "\n";
out << prefix << "MaxLength  = " << MaxLength  << "\n";
out << prefix << "IgnoreCase = " << IgnoreCase << "\n";

int j;

for (j=0; j<Nelements; ++j)  {

   out << prefix2 << "Element # " << j << " = \"" << s[j] << "\"\n";

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::extend(int n)

{

if ( n <= Nalloc )  return;

int k;

k = n/stringarray_alloc_inc;

if ( n%stringarray_alloc_inc )  ++k;

n = k*stringarray_alloc_inc;

char ** u = (char **) 0;

u = new char * [n];

int j;

for (j=0; j<n; ++j)  u[j] = (char *) 0;

if ( s )  {

   for (j=0; j<Nelements; ++j)  u[j] = s[j];

   delete [] s;  s = (char **) 0;

}

s = u;  u = (char **) 0;


Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


const char * StringArray::operator[](int n) const

{

if ( (n < 0) || (n >= Nelements) )  {

   mlog << Error << "\nStringArray::operator[](int) const -> range check error!\n\n";

   exit ( 1 );

}



return ( s[n] );

}


////////////////////////////////////////////////////////////////////////


void StringArray::set_ignore_case(const bool b)

{

IgnoreCase = b;

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::add(const char * text)

{

extend(Nelements + 1);

// Add an empty string for a null pointer
if ( !text ) text = "";

int n = strlen(text);

if ( n > MaxLength )  MaxLength = n;

s[Nelements] = new char [1 + n];

strcpy(s[Nelements], text);

s[Nelements][n] = (char) 0;

++Nelements;



return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::add(const StringArray & a)

{

if ( a.n_elements() == 0 )  return;

extend(Nelements + a.n_elements());

int j;

for (j=0; j<(a.n_elements()); ++j)  {

   add(a[j]);

}


return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::add_css(const char *text)

{

   //
   // Skip parsing for the special case of '*,*' which may appear
   // in the FCST_LEV and OBS_LEV columns of the MET output.
   //

if ( strstr(text, "*,*") != NULL ) {

   add(text);

}
else {

   StringArray sa;

   sa.parse_css(text);

   add(sa);

}

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::set(int i, const char * text)

{

if ( (i < 0) || (i >= Nelements) )  {

   mlog << Error << "\nStringArray::set(int, const char *) -> range check error\n\n";

   exit ( 1 );

}

int n = strlen(text);

if ( n > MaxLength )  MaxLength = n;

if ( s[i] )  { delete [] s[i];  s[i] = (char *) 0; }

s[i] = new char [1 + n];

strcpy(s[i], text);

s[i][n] = (char) 0;


return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::insert(int i, const char * text)

{

if ( (i < 0) || (i > Nelements) )  {

   mlog << Error << "\nStringArray::insert(int, const char *) -> range check error\n\n";

   exit ( 1 );

}

int n = strlen(text);

if ( n > MaxLength )  MaxLength = n;

extend(Nelements + 1);

int j;

for (j=Nelements; j>i; --j) s[j] = s[j-1];

++Nelements;

s[i] = 0;

set(i, text);


return;

}


////////////////////////////////////////////////////////////////////////


int StringArray::has(const char * text) const

{

int index, status;

status = has(text, index);

return ( status );

}


////////////////////////////////////////////////////////////////////////


int StringArray::has(const char * text, int & index) const

{

index = -1;

if ( Nelements == 0 || !text )  return ( 0 );

int j;

for (j=0; j<Nelements; ++j)  {

   if ( IgnoreCase ) {

      if ( strcasecmp(s[j], text) == 0 )  { index = j;  return ( 1 ); }

   }
   else {

      if ( strcmp(s[j], text) == 0 )  { index = j;  return ( 1 ); }

   }

}

   //
   //  nope
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void StringArray::parse_wsss(const char * text)

{

parse_delim(text, " ");

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::parse_css(const char * text)

{

parse_delim(text, ",");

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::parse_delim(const char * text, const char *delim)

{

// check for null pointer
if ( !text ) return;

char * line = (char *) 0;
char * c    = (char *) 0;
char * lp   = (char *) 0;
const int n = strlen(text);

clear();

line = new char [1 + n];

memset(line, 0, 1 + n);

strcpy(line, text);

lp = line;

while ( (c = strtok(lp, delim)) != NULL )  {

   add(c);

   lp = (char *) 0;

}   //  while

   //
   //  done
   //

if ( line )  { delete [] line;  line = (char *) 0; }

lp = (char *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::shift_down(int pos, int shift)

{

if ( (pos < 0) || (pos >= Nelements) )  {

   mlog << Error << "\nStringArray::shift_down() -> bad value for pos\n\n";

   exit ( 1 );

}

if ( (shift <= 0) || ((pos + shift) > Nelements) )  {

   mlog << Error << "\nStringArray::shift_down() -> bad value for shift\n\n";

   exit ( 1 );

}

int j;

for (j=0; j<shift; ++j)  {

   delete [] s[pos + j];  s[pos + j] = (char *) 0;

}

for (j=pos; j<(Nelements - shift); ++j)  {

   s[j] = s[j + shift];

}


for (j=0; j<shift; ++j)  {

   s[Nelements - shift + j] = (char *) 0;

}

Nelements -= shift;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


int StringArray::has_option(int & index) const

{

index = -1;

int j;

for (j=0; j<Nelements; ++j)  {

   if ( s[j][0] == '-' )  {

      index = j;

      return ( 1 );

   }

}

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int StringArray::length(int k) const

{

if ( (k < 0) || (k >= Nelements) )  {

   mlog << Error << "\nStringArray::length(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( strlen(s[k]) );

}


////////////////////////////////////////////////////////////////////////


void StringArray::sort()

{

if ( Nelements <= 1 )  return;

qsort(s, Nelements, sizeof(char *), lex_comp);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


int lex_comp(const void * a, const void * b)

{

int status;
const char ** ca = (const char **) a;
const char ** cb = (const char **) b;


status = strcmp(*ca, *cb);


return ( status );

}


////////////////////////////////////////////////////////////////////////






