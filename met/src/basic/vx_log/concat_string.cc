// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cmath>

using namespace std;

#include "concat_string.h"
#include "logger.h"


////////////////////////////////////////////////////////////////////////


inline int imin(int a, int b)  { return ( (a < b) ? a : b ); }

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
delete s;

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
    if (this != &c) {
        delete s;
        init_from_scratch();
        assign(c);
    }
    return *this;
}


////////////////////////////////////////////////////////////////////////


ConcatString & ConcatString::operator=(const char * Text)
{
    delete s;
    init_from_scratch();
    s->assign(Text);

    return(*this);
}


////////////////////////////////////////////////////////////////////////


void ConcatString::init_from_scratch()
{
    s = new string();
    set_precision(concat_string_default_precision);
}


////////////////////////////////////////////////////////////////////////


void ConcatString::clear()
{
    s->clear();

    set_precision(concat_string_default_precision);
}


////////////////////////////////////////////////////////////////////////


char ConcatString::char_at(const int idx) const
{
   if (0 > idx || s->length() <= idx )
        return '\0';

   return s->at(idx);
}


////////////////////////////////////////////////////////////////////////


void ConcatString::assign(const ConcatString & c)
{
    s->assign(c.text());
    memcpy(FloatFormat, c.FloatFormat, sizeof(FloatFormat));
    Precision = c.Precision;
}


////////////////////////////////////////////////////////////////////////


// void ConcatString::extend(int n)
// {
//
// if ( n < Nalloc )  return;
//
// if ( AllocInc == 0 )  AllocInc = default_cs_alloc_inc;
//
// int k;
//
// k = n/AllocInc;
//
// if ( n%AllocInc )  ++k;
//
// n = k*AllocInc;
//
// char * u = new char [n];
//
// if ( !u )  {
//
//    mlog << Error << "\nConcatString::extend(int) -> memory allocation error\n\n";
//
//    exit ( 1 );
//
// }
//
// memset(u, 0, n);
//
// if ( s && (Length > 0) )  {
//
//    memcpy(u, s, Length);
//
//    delete [] s;  s = (char *) 0;
//
// }
//
// s = u;  u = (char *) 0;
//
// Nalloc = n;
//
// return;
//
// }


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
    if (!a)             // nothing to do for null string
        return;

    (*s) += a;
}


////////////////////////////////////////////////////////////////////////


void ConcatString::add(const char c)
{
    (*s) += c;
}


////////////////////////////////////////////////////////////////////////


void ConcatString::add(const ConcatString & a)
{
    (*s) += (*a.s);
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
    size_t pos = s->find_last_not_of(c);
    if (pos != string::npos)
        s->erase(pos + 1);
    else
        s->clear();
}


////////////////////////////////////////////////////////////////////////


void ConcatString::chomp(const char * suffix)
{
    size_t limit = s->length() - strlen(suffix);
    size_t pos = s->find(suffix, limit);
    if (pos != string::npos)
        s->erase(pos);
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

snprintf(FloatFormat, sizeof(FloatFormat), "%%.%df", Precision);

return;

}


////////////////////////////////////////////////////////////////////////


void ConcatString::erase()
{
    s->clear();
}


////////////////////////////////////////////////////////////////////////


void ConcatString::set_repeat(char c, int count)
{
    s->assign(count, c);
}


////////////////////////////////////////////////////////////////////////


void ConcatString::elim_trailing_whitespace()
{
    // This will work with the standard "C" locale. Others may require a different char set
    s->erase(s->find_last_not_of(" \n\r\t\v\f") + 1);
}


////////////////////////////////////////////////////////////////////////


bool ConcatString::startswith(const char * Text) const
{
    size_t pos = s->rfind(Text, strlen(Text));
    return (pos != string::npos);
}


////////////////////////////////////////////////////////////////////////


bool ConcatString::endswith(const char * Text) const
{
    size_t pos = s->find(Text, s->length() - strlen(Text));
    return (pos != string::npos);
}


////////////////////////////////////////////////////////////////////////


StringArray ConcatString::split(const char * delim) const
{
    StringArray a;
    if (s->empty()) {
        return a;
    }

    size_t start = 0;
    size_t end = s->find_first_of(delim);
    while (end != string::npos) {
        if (start != end)
            a.add(s->substr(start, end-start).c_str());
        start = end + 1;
        end = s->find_first_of(delim, start);
    }
    if (start < s->length())
        a.add(s->substr(start).c_str());

    return a;
}


////////////////////////////////////////////////////////////////////////


void ConcatString::ws_strip()
{
    // This will work with the standard "C" locale.
    // Other locales may require a different whitespace char set.
    const char * ws = " \n\r\t\v\f";

    s->erase(0, s->find_first_not_of(ws));
    s->erase(s->find_last_not_of(ws) + 1);
}


////////////////////////////////////////////////////////////////////////


void ConcatString::strip_cpp_comment()
{
    size_t pos = s->find("//");
    if (pos != string::npos)
        s->erase(pos);
}


////////////////////////////////////////////////////////////////////////


int ConcatString::format(const char *fmt, ...)
{
    va_list vl;
    int status = -1;
    char *tmp;

    va_start(vl, fmt);
    status = vasprintf(&tmp, fmt, vl);

    if (status == -1) {
       mlog << Error << "\nConcatString::format() could not allocate a temporary buffer.\n\n";
       exit(1);
    }

    s->assign(tmp);
    free(tmp);
    va_end(vl);
    return status;
}


////////////////////////////////////////////////////////////////////////


void ConcatString::replace(const char * target, const char * replacement,
                           bool check_env)
{
    if (empty())
        return;

    if (::is_empty(target) || ::is_empty(replacement) )  {
       mlog << Error << "\nConcatString::replace(const char * target, const char * replacement, bool check_env) -> target and/or replacement string is empty\n\n";
       exit ( 1 );
    }

    const char * c = (const char *) 0;
    if (check_env && (c = getenv(replacement)) != NULL)
        replacement = c;


    size_t pos;
    while ((pos = s->find(target)) != string::npos) {
        s->replace(pos, strlen(target), replacement);
    }
}


////////////////////////////////////////////////////////////////////////


void ConcatString::set_upper()
{
    for (string::iterator c = s->begin(); s->end() != c; ++c)
        *c = toupper(*c);
}


////////////////////////////////////////////////////////////////////////


void ConcatString::set_lower()
{
    for (string::iterator c = s->begin(); s->end() != c; ++c)
        *c = tolower(*c);
}


////////////////////////////////////////////////////////////////////////


const char * ConcatString::contents(const char *str) const
{
    if (s->empty()) {
        return (str ? str : "(nul)");
    } else {
        return (s->c_str());
    }
}


////////////////////////////////////////////////////////////////////////


bool ConcatString::read_line(istream & in)
{
    erase();
    getline(in, *s);
    if (!in) {
          // Check for end of file and non-empty line
          if (in.eof() && (s->length() != 0))
            return true;
          else
            return false;
    }

    return true;
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


char ConcatString::operator[](const int n) const
{
    if ((n < 0) || (n >= s->length()))  {
        mlog << Error << "\nConcatString::operator[](const int) const -> range check error\n\n";
        exit ( 1 );
    }

    return(s->at(n));
}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & cs, const char c)

{

cs.add(c);

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

snprintf(junk, sizeof(junk), "%d", k);

a.add(junk);


return ( a );

}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, long long k)

{

char junk[128];

snprintf(junk, sizeof(junk), "%lld", k);

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


ostream & operator<<(ostream & out, const ConcatString & c)
{
    if (c.length()) {
        out << c.text();
    }

    return out;
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


ConcatString write_css(const StringArray &sa)

{

ConcatString css;

for ( int i=0; i<sa.n_elements(); ++i )  {
   css << (i == 0 ? "" : ",") << sa[i];
}

return(css);

}


////////////////////////////////////////////////////////////////////////


bool is_empty(const char * text)

{

return ( (text == NULL) || (*text == 0) || (strlen(text) == 0));

}


////////////////////////////////////////////////////////////////////////


