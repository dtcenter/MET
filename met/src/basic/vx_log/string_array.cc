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
#include <regex.h>
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


bool StringArray::operator==(const StringArray & a) const

{

if ( n() != a.n() )  return ( false );

int j;

for (j=0; j<n(); ++j)  {

  if ( s[j] != a.s[j] )  return ( false );

}

return ( true );

}


////////////////////////////////////////////////////////////////////////


void StringArray::init_from_scratch()

{

IgnoreCase = 0;

clear();




return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::clear()

{
s.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::assign(const StringArray & a)

{

clear();

s = a.s;

IgnoreCase = a.IgnoreCase;

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::dump(ostream & out, int depth) const

{

Indent prefix(depth);
Indent prefix2(depth + 1);

out << prefix << "IgnoreCase = " << IgnoreCase << "\n";

int j;

for (j=0; j<n(); ++j)  {

   out << prefix2 << "Element # " << j << " = \"" << s[j] << "\"\n";

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


const std::string StringArray::operator[](int len) const

{

  if ( (len < 0) || (len >= n()) )  {

    mlog << Error << "\nStringArray::operator[](int) const -> range check error!\n\n";

   exit ( 1 );

}



return ( s[len] );

}


////////////////////////////////////////////////////////////////////////




void StringArray::set_ignore_case(const bool b)

{

IgnoreCase = b;

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::add(const std::string text)

{

s.push_back(text);

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::add(const StringArray & a)

{

if ( a.n() == 0 )  return;

s.insert(s.end(), a.s.begin(), a.s.end());
 
return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::add_css(const std::string text)

{

   //
   // Skip parsing for the special case of '*,*' which may appear
   // in the FCST_LEV and OBS_LEV columns of the MET output.
   //

  if (text.find("*,*") != std::string::npos) {

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


void StringArray::set(int i, const std::string text)

{

  if ( (i < 0) || (i >= n()) )  {

   mlog << Error << "\nStringArray::set(int, const string) -> range check error\n\n";

   exit ( 1 );

}

s[i] = text;

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::insert(int i, const char * text)

{

  if ( (i < 0) || (i > n()) )  {

    mlog << Error << "\nStringArray::insert(int, const char *) -> range check error\n\n";

    exit ( 1 );

  }

  s.insert(s.begin()+i, text);

  return;

}


////////////////////////////////////////////////////////////////////////


bool StringArray::has(const std::string text) const

{

  int index;

  return ( has(text, index) );

}


////////////////////////////////////////////////////////////////////////


bool StringArray::has(const std::string text, int & index) const

{

  index = -1;

  std::string str = text;

  std::vector<std::string>::const_iterator it;
  int count = 0;

  for(it = s.begin(); it != s.end(); it++, count++) {
    if ( IgnoreCase ) {
      std::string lower_s = *it;
      transform(lower_s.begin(), lower_s.end(), lower_s.begin(), ::tolower);
      std::string lower_text = text;
      transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
      if ( lower_s == lower_text) {
	//      if ( strcasecmp((*it).c_str(), text.c_str()) ) {
        index = count;
	break;
      }
    }
    else {
      if ( *it == text ) {
        index = count;
	break;
      }
    }
  }

  if (it != s.end()) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////////////////


void StringArray::parse_wsss(const std::string text)

{

parse_delim(text, " ");

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::parse_css(const std::string text)

{

parse_delim(text, ",");

return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::parse_delim(const std::string text, const char *delim)

{

  clear();

  std::string str = text;
    
  size_t start = 0;
  size_t end = str.find_first_of(delim);
  while (end != string::npos) {
      if (start != end)
          s.push_back(str.substr(start, end-start).c_str());
      start = end + 1;
      end = str.find_first_of(delim, start);
  }
  if (start < str.length())
      s.push_back(str.substr(start).c_str());

  return;

}


////////////////////////////////////////////////////////////////////////


void StringArray::shift_down(int pos, int shift)

{

if ( (pos < 0) || (pos >= n()) )  {

   mlog << Error << "\nStringArray::shift_down() -> bad value for pos\n\n";

   exit ( 1 );

}

if ( (shift <= 0) || ((pos + shift) > n()) )  {

   mlog << Error << "\nStringArray::shift_down() -> bad value for shift\n\n";

   exit ( 1 );

}

 s.erase(s.begin() + pos, s.begin() + pos + shift);

 return;

}


////////////////////////////////////////////////////////////////////////


bool StringArray::has_option(int & index) const

{

index = -1;

int j;

for (j=0; j<n(); ++j)  {

   if ( s[j][0] == '-' )  {

      index = j;

      return ( true );

   }

}

return ( false );

}



////////////////////////////////////////////////////////////////////////


bool StringArray::reg_exp_match(const char * text) const

{

if ( n() == 0 || !text )  return ( false );

int j;

 for (j=0; j<n(); ++j)  {

   if ( check_reg_exp(s[j].c_str(), text) )  { return ( true ); }

}

   //
   //  nope
   //

return ( false );

}


////////////////////////////////////////////////////////////////////////


int StringArray::length(int k) const

{

if ( (k < 0) || (k >= n()) )  {

   mlog << Error << "\nStringArray::length(int) const -> range check error\n\n";

   exit ( 1 );

}

return ( s[k].length() );

}


////////////////////////////////////////////////////////////////////////


void StringArray::sort()

{

if ( n() <= 1 )  return;

 std::sort(s.begin(), s.end());
 
return;

}


////////////////////////////////////////////////////////////////////////


StringArray StringArray::uniq()

{

  StringArray sa;

  sa.s = s;
  
  std::vector<std::string>::iterator it;

  it = std::unique(sa.s.begin(), sa.s.end());

  sa.s.resize(std::distance(sa.s.begin(), it));

  return sa;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


bool check_reg_exp(const char *reg_exp_str, const char *test_str)

{

bool valid = false;
regex_t buffer;
regex_t *preg = &buffer;

// Check for null pointers
if( !reg_exp_str || !test_str ) return( false ); 

if( regcomp(preg, reg_exp_str, REG_EXTENDED*REG_NOSUB) != 0 ) {
   mlog << Error << "\ncheck_reg_exp(char *, char *) -> "
        << "regcomp error for \""
        << reg_exp_str << "\" and \"" << test_str << "\"\n\n";

   exit ( 1 );
}

if( regexec(preg, test_str, 0, 0, 0) == 0 ) { valid = true; }

// Free allocated memory.
regfree( preg );

return( valid );

}


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






