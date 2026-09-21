// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cmath>

#include "info.h"
#include "str_wrappers.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class EnumInfo
   //


////////////////////////////////////////////////////////////////////////


EnumInfo::EnumInfo()

{

init_from_scratch();

clear();

}


////////////////////////////////////////////////////////////////////////


EnumInfo::~EnumInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


EnumInfo::EnumInfo(const EnumInfo & e)

{

init_from_scratch();

assign(e);

}


////////////////////////////////////////////////////////////////////////


EnumInfo & EnumInfo::operator=(const EnumInfo & e)

{

if ( this == &e )  return *this;

assign(e);

return *this;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::init_from_scratch()

{

clear();


return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::clear()

{

   //
   //  NOTE: this began with "if ( !s ) return;", a guard against walking a
   //  null id array. That also skipped clearing Name/Scope/Header whenever an
   //  EnumInfo had no ids. The containers clean themselves up, so the guard is
   //  gone and clear() now clears everything.
   //

s.clear();

Name.clear();

LowerCaseName.clear();

Scope.clear();

U_Scope.clear();

Header.clear();


return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::assign(const EnumInfo & e)

{

clear();

s = e.s;

set_name(e.Name.c_str());

set_header(e.Header.c_str());

set_scope(e.Scope.c_str());


return;

}


////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////


const char * EnumInfo::id(int n) const

{

if ( (n < 0) || (n >= (int) s.size()) )  {

   cerr << "\n\n  EnumInfo::id(int) -> range check error\n\n";

   exit ( 1 );

}

return s[n].c_str();

}


////////////////////////////////////////////////////////////////////////


int EnumInfo::max_id_length() const

{

if ( s.empty() )  return 0;

int j, k;
int max_len;

max_len = 0;

for (j=0; j<(int) s.size(); ++j)  {

   k = (int) s[j].length();

   if ( k > max_len )  max_len = k;

}


return max_len;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::add_id(const char * text)

{

s.push_back(text ? text : "");


return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::set_name(const char * text)

{

   //  both are cleared BEFORE the null check, as the original did

Name.clear();

LowerCaseName.clear();

if ( !text )  return;

Name = text;

LowerCaseName = text;

for (char & c : LowerCaseName)  c = tolower((unsigned char) c);


return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::set_scope(const char * text)

{

Scope.clear();

U_Scope.clear();

if ( !text )  return;

Scope = text;

for (int j=0; j<(int) Scope.length(); ++j)  {

   const char c = Scope[j];

   if ( c == ':' )  { ++j;  U_Scope += '_'; }
   else             U_Scope += c;

}


return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::set_header(const char * text)

{

   //  a null text previously yielded an empty string, which clear() reproduces

Header.clear();

if ( !text )  return;

Header = text;


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & s, const EnumInfo & e)

{

int j;
const char * c = "(nul)";


if ( !e.Name.empty() )  c = e.Name.c_str();

s << "enum " << c << " from header file ";

c = "(nul)";

if ( !e.Header.empty() )  c = e.Header.c_str();

s << c << "\n";

s << "There are " << (e.s.size()) << " ids\n";

for (j=0; j<(int) (e.s.size()); ++j)  {

   s << "    " << j << "   \"";

   s << e.id(j);

   s << "\"\n";

}




s.flush();

return s;

}


////////////////////////////////////////////////////////////////////////




