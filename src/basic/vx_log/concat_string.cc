// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cmath>

#include "concat_string.h"
#include "logger.h"


////////////////////////////////////////////////////////////////////////


static bool is_empty(const char *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ConcatString
   //


////////////////////////////////////////////////////////////////////////


ConcatString::ConcatString()

{

init_from_scratch();

set_alloc_inc(default_cs_alloc_inc);

}


////////////////////////////////////////////////////////////////////////


ConcatString::ConcatString(int _alloc_inc)

{

init_from_scratch();

set_alloc_inc(_alloc_inc);

}


////////////////////////////////////////////////////////////////////////


ConcatString::~ConcatString()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ConcatString::ConcatString(const ConcatString & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


ConcatString::ConcatString(const char * Text)

{

init_from_scratch();

add(Text);

}


////////////////////////////////////////////////////////////////////////


ConcatString & ConcatString::operator=(const ConcatString & c)

{

if ( this == &c )  return ( * this );

assign(c);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


ConcatString & ConcatString::operator=(const char * Text)

{

if ( s == Text )  return ( * this );

clear();

add(Text);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ConcatString::init_from_scratch()

{

s = (char *) 0;

clear();

AllocInc = default_cs_alloc_inc;

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::clear()

{

if ( s )  { delete [] s;  s = (char *) 0; }

Length = Nalloc = 0;

set_precision(concat_string_default_precision);

   //
   //  we do NOT reset AllocInc in this function
   //

return;

}


////////////////////////////////////////////////////////////////////////


char ConcatString::char_at(const int idx) const

{

   if( 0 > idx || Length <= idx ) return '\0';

   return s[idx];

}


////////////////////////////////////////////////////////////////////////


void ConcatString::assign(const ConcatString & c)

{

clear();

if ( !(c.s) )  return;

extend(1 + c.Length);

memcpy(s, c.s, c.Length);

Length = c.Length;

Precision = c.Precision;

memcpy(FloatFormat, c.FloatFormat, sizeof(FloatFormat));

   //
   //  we do NOT copy c.AllocInc
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::extend(int n)

{

if ( n < Nalloc )  return;

int k;

k = n/AllocInc;

if ( n%AllocInc )  ++k;

n = k*AllocInc;

char * u = (char *) 0;

u = new char [n];

if ( !u )  {

   mlog << Error << "\nConcatString::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, n);

if ( s && (Length > 0) )  {

   memcpy(u, s, Length);

   delete [] s;  s = (char *) 0;

}

s = u;  u = (char *) 0;

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::set_alloc_inc(int _alloc_inc)

{

if ( _alloc_inc < min_cs_alloc_inc )  _alloc_inc = min_cs_alloc_inc;

AllocInc = _alloc_inc;

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::add(const char * a)

{

if(!a) return;            // nothing to do for null string

int n = strlen(a);

extend(Length + n + 5);   //  just to make sure

strcpy(s + Length, a);

Length += n;


return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::add(const char c)

{

extend(Length + 5);   //  just to make sure

s[Length++] = c;

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::add(const ConcatString & a)

{

add(a.text());

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::chomp()

{

chomp('\n');

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::chomp(const char c)

{

if ( Length == 0 )  return;

if ( s[Length - 1] == c )  {

   --Length;

   s[Length] = (char) 0;

}



return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::chomp(const char * suffix)

{

const int N = strlen(suffix);

if ( N > Length )  return;

int j;

if ( strncmp(s + (Length - N), suffix, N) == 0 )  {

   for (j=0; j<N; ++j)  {

      s[Length - N + j] = (char) 0;

   }

   Length -= N;

}


return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::set_precision(int k)

{

if ( (k < 0) || (k > concat_string_max_precision) )  {

   mlog << Error << "\nConcatString::set_precision(int) -> bad value\n\n";

   exit ( 1 );

}


Precision = k;

memset(FloatFormat, 0, sizeof(FloatFormat));

sprintf(FloatFormat, "%%.%df", Precision);

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::erase()

{

if ( !s )  return;

memset(s, 0, Nalloc);

Length = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::set_repeat(char c, int count)

{

extend(count + 1);

memset(s, 0, Nalloc);

memset(s, c, count);

Length = ( (c == 0) ? 0 : count );


return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::elim_trailing_whitespace()

{

if ( !s || (Length == 0) )  return;


int j = Length - 1;


while ( j>=0 )  {

   if ( !(isspace(s[j])) )  break;

   s[j] = (char) 0;

   --Length;  --j;

}   //  for j

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool ConcatString::startswith(const char * Text) const

{

if ( !s )  return ( false );

int n = strlen(Text);

if ( Length < n )  return ( false );

if ( strncmp(s, Text, n) == 0 )  return ( true );

return ( false );

}


////////////////////////////////////////////////////////////////////////


bool ConcatString::endswith(const char * Text) const

{

if ( !s )  return ( false );

int n = strlen(Text);

if ( Length < n )  return ( false );

if ( strncmp(s + (Length - n), Text, n) == 0 )  return ( true );

return ( false );

}


////////////////////////////////////////////////////////////////////////


StringArray ConcatString::split(const char * delim) const

{

StringArray a;

if ( (!s) || (Length == 0) )  return ( a );

char * u = (char *) 0;

u = new char [1 + Length];

if ( !u )  {

   mlog << Error << "\nConcatString::split(const char *) const -> memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, 1 + Length);

strncpy(u, s, Length);

const char * c = (const char *) 0;
char * line = (char *) 0;

line = u;

while ( 1 )  {

   c = strtok(line, delim);

   if ( !c )  break;

   a.add(c);

   line = (char *) 0;

}   //  while

   //
   //  done
   //

if ( u )  { delete [] u;  u = (char *) 0; }

return ( a );

}


////////////////////////////////////////////////////////////////////////


void ConcatString::ws_strip()

{

if ( !s || (Length == 0) )  return;

char * u = (char *) 0;
int start, end;

u = new char [1 + Length];

if ( !u )  {

   mlog << Error << "\nConcatString::ws_strip() -> memory allocation error\n\n";

   exit ( 1 );

}

memset(u, 0, 1 + Length);

strncpy(u, s, Length);

   //
   //  find start of non-whitespace
   //

for (start=0; start<Length; ++start)  {

   if ( !isspace(s[start]) )  break;

}

if ( start == Length )  { erase();  return; }

   //
   //  find end of non-whitespace
   //

for (end=(Length - 1); end>=0; --end)  {

   if ( !isspace(s[end]) )  break;

}

if ( end < 0 )  { erase();  return; }

   //
   //  delimit the string and assign it to this
   //

u[end + 1] = (char) 0;

*this = (u + start);

   //
   //  done
   //

if ( u )  { delete [] u;  u = (char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::strip_cpp_comment()

{

if ( Length == 0 )  return;

int pos;
char * c = (char *) 0;

c = strstr(s, "//");

if ( !c )  return;

pos = (int) (c - s);

memset(c, 0, Nalloc - pos);

Length = strlen(s);

return;

}


////////////////////////////////////////////////////////////////////////


int ConcatString::format(const char *fmt, ...)

{

va_list vl;
int status = -1;

extend(max_str_len);

va_start(vl, fmt);

status = vsprintf(s, fmt, vl);

if( status >= Nalloc - 2 ){

   mlog << Error << "\nConcatString::format() -> overwrote buffer.\n\n";

   exit(1);

}

va_end(vl);

Length = strlen(s);

return status;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::replace(const char * target, const char * replacement,
                           bool check_env)

{

if ( empty() )  return;


if ( ::is_empty(target) || ::is_empty(replacement) )  {

   mlog << Error << "\nConcatString::replace(const char * target, const char * replacement, bool check_env) -> target and/or replacement string is empty\n\n";

   exit ( 1 );

}

const char * c = (const char *) 0;

if ( check_env && (c = getenv(replacement)) != NULL )  replacement = c;


const int target_len = strlen(target);
ConcatString A;
const char * S = s;
const char * end = s + Length;

while ( S < end )  {

   if ( (c = strstr(S, target)) != NULL )  {

      while ( S < c )  A.add(*S++);

      A << replacement;

      S += target_len;

   } else {

      while ( S < end )  A.add(*S++);

   }

}

if ( A.nonempty() )  assign(A);


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::set_upper()

{
  
for (int i=0; i<Length; ++i)  s[i] = toupper(s[i]);

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::set_lower()

{

for (int i=0; i<Length; ++i)  s[i] = toupper(s[i]);

return;

}


////////////////////////////////////////////////////////////////////////


const char * ConcatString::contents() const

{

if ( Length == 0 )  return ( "(nul)" );

return ( s );

}


////////////////////////////////////////////////////////////////////////


bool ConcatString::read_line(istream & in)

{

char c;


erase();

do {

   in.get(c);

   if ( !in )  return ( false );

   add(c);

} while ( c != '\n' );


return ( true );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ConcatString to_upper(const ConcatString &cs)

{

ConcatString uc = cs;

uc.set_upper();

return ( uc );

}

////////////////////////////////////////////////////////////////////////


ConcatString to_lower(const ConcatString &cs)

{

ConcatString lc = cs;

lc.set_lower();

return ( lc );

}

////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & cs, const char c)

{

char s[2];

s[0] = c;
s[1] = (char) 0;

cs.add(s);

return ( cs );

}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & cs, const char * s)

{

cs.add(s);

return ( cs );

}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, const ConcatString & b)

{

a.add(b);

return ( a );

}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, int k)

{

char junk[128];

sprintf(junk, "%d", k);

a.add(junk);


return ( a );

}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, long long k)

{

char junk[128];

sprintf(junk, "%lld", k);

a.add(junk);


return ( a );

}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, double x)

{

char junk[512];

snprintf(junk, sizeof(junk), a.float_format(), x);

a.add(junk);


return ( a );

}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, CSInlineCommand c)

{

switch ( c )  {

   case cs_erase:  a.erase();  break;
   case cs_clear:  a.clear();  break;

   default:
      mlog << Error << "\noperator<<(ostream &, CSInlineCommand) -> bad CSInlineCommand value\n\n";
      exit ( 1 );
      break;

}   //  switch


return ( a );

}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, const Indent & i)

{

int j, jmax;


jmax = (i.delta)*(i.depth);


for (j=0; j<jmax; ++j)  {

   if ( (j%(i.delta)) == 0 )  a << i.on_char;
   else                       a << i.off_char;

}


return ( a );

}


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & out, const ConcatString & s)

{

out << (s.text());

return ( out );

}


////////////////////////////////////////////////////////////////////////


bool operator==(const ConcatString & a, const ConcatString & b)

{

if ( a.empty() )  return ( false );
if ( b.empty() )  return ( false );

int status = strcmp(a.text(), b.text());

return ( status == 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator==(const ConcatString & a, const char * text)

{

if ( !text || a.empty() )  return ( false );

int status = strcmp(text, a.text());

return ( status == 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator==(const char * text, const ConcatString & a)

{

if ( !text || !a.text() )  return ( false );

int status = strcmp(text, a.text());

return ( status == 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator!=(const ConcatString & a, const ConcatString & b)

{
	return ( !(a == b) );
}


////////////////////////////////////////////////////////////////////////


bool operator!=(const ConcatString & a, const char * text)

{
	return ( !(a == text) );
}


////////////////////////////////////////////////////////////////////////


bool operator!=(const char * text, const ConcatString & a)

{
	return ( !(text == a) );
}


////////////////////////////////////////////////////////////////////////


bool operator>=(const ConcatString & a, const ConcatString & b)

{

if ( a.empty() )  return ( false );
if ( b.empty() )  return ( false );

int status = strcmp(a.text(), b.text());

return ( status >= 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator>=(const ConcatString & a, const char * text)

{

if ( a.empty() || !text )  return ( false );

int status = strcmp(a.text(), text);

return ( status >= 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator>=(const char * text, const ConcatString & a)

{

if ( a.empty() || !text )  return ( false );

int status = strcmp(text, a.text());

return ( status >= 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator<=(const ConcatString & a, const ConcatString & b)

{

if ( a.empty() )  return ( false );
if ( b.empty() )  return ( false );

int status = strcmp(a.text(), b.text());

return ( status <= 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator<=(const ConcatString & a, const char * text)

{

if ( a.empty() || !text )  return ( false );

int status = strcmp(a.text(), text);

return ( status <= 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator<=(const char * text, const ConcatString & a)

{

if ( a.empty() || !text )  return ( false );

int status = strcmp(text, a.text());

return ( status <= 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator>(const ConcatString & a, const ConcatString & b)

{

if ( a.empty() )  return ( false );
if ( b.empty() )  return ( false );

int status = strcmp(a.text(), b.text());

return ( status > 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator>(const ConcatString & a, const char * text)

{

if ( a.empty() || !text )  return ( false );

int status = strcmp(a.text(), text);

return ( status > 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator>(const char * text, const ConcatString & a)

{

if ( a.empty() || !text )  return ( false );

int status = strcmp(text, a.text());

return ( status > 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator<(const ConcatString & a, const ConcatString & b)

{

if ( a.empty() )  return ( false );
if ( b.empty() )  return ( false );

int status = strcmp(a.text(), b.text());

return ( status < 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator<(const ConcatString & a, const char * text)

{

if ( a.empty() || !text )  return ( false );

int status = strcmp(a.text(), text);

return ( status < 0 );

}


////////////////////////////////////////////////////////////////////////


bool operator<(const char * text, const ConcatString & a)

{

if ( a.empty() || !text )  return ( false );

int status = strcmp(text, a.text());

return ( status < 0 );

}


////////////////////////////////////////////////////////////////////////


bool is_empty(const char * text)

{

return ( (text == NULL) || (*text == 0) );

}


////////////////////////////////////////////////////////////////////////


