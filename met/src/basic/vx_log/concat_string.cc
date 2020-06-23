// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
: Precision(0)
{

init_from_scratch();

set_alloc_inc(default_cs_alloc_inc);

}


////////////////////////////////////////////////////////////////////////


ConcatString::ConcatString(int _alloc_inc)
: Precision(0)
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
: Precision(0)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


ConcatString::ConcatString(const std::string & Text)
: Precision(0)

{

init_from_scratch();

add(Text);

}


////////////////////////////////////////////////////////////////////////


ConcatString::ConcatString(const char * Text)
: Precision(0)

{

if ( ! Text )  {

   mlog << Error << "\nConcatString::ConcatString(const char *) -> "
        << "null pointer!\n\n";

   exit ( 1 );

}

init_from_scratch();

add(::string(Text));

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


ConcatString & ConcatString::operator=(const std::string & Text)

{
   delete s;
   init_from_scratch();
   if (s) s->assign(Text);

   return(*this);
}


////////////////////////////////////////////////////////////////////////


ConcatString & ConcatString::operator=(const char * Text)

{


if ( ! Text )  {

   mlog << Error << "\nConcatString::operator=(const char *) -> "
        << "null pointer!\n\n";

   exit ( 1 );

}

   std::string s2 = Text;
   if ( s )  delete s;
   init_from_scratch();

   (*s) = s2;

   return(*this);
}


////////////////////////////////////////////////////////////////////////


void ConcatString::init_from_scratch()
{
   s = new std::string();
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
   if (0 > idx || length() <= idx ) return '\0';

   return s->at(idx);
}


////////////////////////////////////////////////////////////////////////


void ConcatString::assign(const ConcatString & c)
{
   if (c.text()) s->assign(c.text());
   else          s->clear();

   memcpy(FloatFormat, c.FloatFormat, sizeof(FloatFormat));
   Precision = c.Precision;
}


////////////////////////////////////////////////////////////////////////


void ConcatString::set_alloc_inc(int _alloc_inc)

{

if ( _alloc_inc < min_cs_alloc_inc )  _alloc_inc = min_cs_alloc_inc;

AllocInc = _alloc_inc;

return;

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


void ConcatString::add(const std::string & a)
{
   (*s) += a;
}



////////////////////////////////////////////////////////////////////////


void ConcatString::add(const char * Text)
{

if ( !Text )  {

   mlog << Error << "\nConcatString::add(const char *) -> "
        << "null pointer\n\n";

   exit ( 1 );

}

   (*s) += ::string(Text);
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
   if (pos != string::npos) s->erase(pos + 1);
   else                     s->clear();
}


////////////////////////////////////////////////////////////////////////


void ConcatString::chomp(const char * suffix)
{
   size_t limit = length() - strlen(suffix);
   size_t pos = s->find(suffix, limit);
   if (pos != string::npos) s->erase(pos);
}


////////////////////////////////////////////////////////////////////////


void ConcatString::set_precision(int k)

{

if ( (k < 0) || (k > concat_string_max_precision) )  {

   mlog << Error << "\nConcatString::set_precision(int) -> bad value\n\n";

   exit ( 1 );

}

if (Precision != k) {
   Precision = k;

   memset(FloatFormat, 0, sizeof(FloatFormat));

   snprintf(FloatFormat, sizeof(FloatFormat), "%%.%df", Precision);
}

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
           a.add(s->substr(start, end-start));
       start = end + 1;
       end = s->find_first_of(delim, start);
   }
   if (start < s->length())
       a.add(s->substr(start));

   return a;
}


////////////////////////////////////////////////////////////////////////


ConcatString ConcatString::dirname() const
{
   ConcatString c;

   // Delete trailing slash, if present
   c = *this;
   c.chomp("/");

   // Find last forward slash in the string
   size_t start = 0;
   size_t end   = c.s->find_last_of("/");

   // No forward slashes found
   if (end == string::npos)  {
      c = ".";
   }
   // Copy up to the last forward slash
   else  {
      c = c.s->substr(start, end-start);
   }

   return(c);
}


////////////////////////////////////////////////////////////////////////


ConcatString ConcatString::basename() const
{
   ConcatString c;

   // Delete trailing slash, if present
   c = *this;
   c.chomp("/");
   
   // Find last forward slash in the string
   size_t start = c.s->find_last_of("/");
   size_t end   = c.s->length();

   // No forward slashes found
   if (start == string::npos)  {
      c = c.s->c_str();
   }
   // Copy from the last forward slash to the end
   else  {
      c = c.s->substr(start+1, end-start);
   }

   return(c);
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


void ConcatString::strip_paren()
{
   size_t pos = s->find("(");
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


void ConcatString::replace_char(int i, char c)
{
  s->replace(i, 1, 1, c);
}


////////////////////////////////////////////////////////////////////////


void ConcatString::replace(const char * target, const char * replacement,
                          bool check_env)
{
   if (empty())
       return;

   if (::is_empty(target) || ::is_empty(replacement) ) {
      mlog << Error << "\nConcatString::replace(const char * target, const char * replacement, bool check_env) -> "
           << "target and/or replacement string is empty\n\n";
      exit ( 1 );
   }

   ConcatString repl_env;
   if (check_env && get_env(replacement, repl_env)) {
      replacement = repl_env.c_str();
   }

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


const string ConcatString::contents(const char * str) const
{
   if (s->empty() || *s == "") {
      return (str ? str : "(nul)");
   } else {
      return ( *s );
   }
}


////////////////////////////////////////////////////////////////////////


bool ConcatString::read_line(istream & in)
{
   erase();
   getline(in, *s);
   if (!in) {
      // Check for end of file and non-empty line
      if (in.eof() && (length() != 0))
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
   if ((n < 0) || (n >= length()))  {
      mlog << Error << "\nConcatString::operator[](const int) const -> "
           << "range check error\n\n";
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


ConcatString & operator<<(ConcatString & cs, const std::string & s)

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

std::ostringstream sstream;
sstream << k;
a.add(sstream.str());

return ( a );
}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, unsigned int k)

{

std::ostringstream sstream;
sstream << k;
a.add(sstream.str());

return ( a );
}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, long long k)

{

std::ostringstream sstream;
sstream << k;
a.add(sstream.str());

return ( a );

}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, double x)

{
std::ostringstream sstream;

sstream.setf(std::ios::fixed, std:: ios::floatfield);
sstream.precision(a.precision());
sstream << x;

a.add(sstream.str());

return ( a );

}


////////////////////////////////////////////////////////////////////////


ConcatString & operator<<(ConcatString & a, CSInlineCommand c)

{

switch ( c )  {

   case cs_erase:  a.erase();  break;
   case cs_clear:  a.clear();  break;

   default:
     mlog << Error << "\noperator<<(ostream &, CSInlineCommand) -> "
          << "bad CSInlineCommand value\n\n";
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


bool ConcatString::operator==(const ConcatString & b) const

{

if ( empty() )  return ( false );
if ( b.empty() )  return ( false );

int status = s->compare(*(b.s));

return ( status == 0 );

}


////////////////////////////////////////////////////////////////////////


bool ConcatString::operator==(const char * text) const

{

if ( !text || empty() )  return ( false );

int status = s->compare(text);

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


bool get_env(const char *env_name, ConcatString &env_value)

{

const char *ptr;
string str = env_name;
static const char *method_name = "get_env() ";

// Initialize
env_value.clear();

if (str.find('/') != string::npos ||
   (ptr = getenv(env_name)) == NULL) {
   return(false);
}

env_value = ptr;
str = env_value;

mlog << Debug(10) << method_name
     << " " << env_name << " to " << env_value << "\n";

int count_replaced = 0;
string nested_value;
size_t pos, pos_end, pos_env, pos_env_end;
pos = pos_env_end = 0;
while ((pos = str.find('$', pos)) != string::npos) {
   string nested_name;
   pos_env = pos + 1;
   if ('{' == str.at(pos_env)) {
      pos_env++;
      pos_end = str.find('}', pos);
      if (string::npos == pos_end) {
         mlog << Error << "\n" << method_name << "\""
              << str << "\" The right curly bracket is missing.\n\n";
         exit (1);
      }
      else {
         pos_env_end = pos_end;
         pos_end++;
      }
   }
   else {
      pos_end = str.find('/', pos);
      pos_env_end = pos_end - 1;
      if (string::npos == pos_end) {
         pos_end = str.length();
         pos_env_end = pos_end;
      }
   }
   nested_name = str.substr(pos_env, (pos_env_end-pos_env));
   if((ptr = getenv(nested_name.c_str())) == NULL) {
      mlog << Error << "\n" << method_name
           << "can't get value of nested environment variable \""
           << nested_name << "\" from " << env_name << "\n\n";
      exit ( 1 );
   }
   nested_value = ptr;
   str.replace(pos, (pos_end - pos), nested_value);
   mlog << Debug(7) << method_name << " " << nested_name
        << " to " << nested_value << "\n";
   count_replaced++;
}

if (count_replaced > 0) {
   env_value = str;
   mlog << Debug(5) << method_name
        << env_name << " to \"" << env_value << "\"\n";
}

return(true);

}


////////////////////////////////////////////////////////////////////////


int ConcatString::find(int c)

{
  std::string::size_type position = s->rfind(c);
  if ( position != std::string::npos) {
     return position;
  }
  else {
     return -1;
  }
}


////////////////////////////////////////////////////////////////////////


int ConcatString::compare(size_t pos, size_t len, std::string str)

{
   return s->compare(pos, len, str);
}


////////////////////////////////////////////////////////////////////////


int ConcatString::comparecase(size_t pos, size_t len, std::string str)

{
   std::string lower_s = *s;
   transform(lower_s.begin(), lower_s.end(), lower_s.begin(), ::tolower);
   std::string lower_str = str;
   transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
   return lower_s.compare(pos, len, lower_str);
}


////////////////////////////////////////////////////////////////////////


int ConcatString::comparecase(const char * str)

{
  std::string lower_s = *s;
  transform(lower_s.begin(), lower_s.end(), lower_s.begin(), ::tolower);
  std::string lower_str = str;
  transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
  return lower_s.compare(lower_str);
}


////////////////////////////////////////////////////////////////////////
