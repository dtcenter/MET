// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
#include <ctype.h>
#include <cmath>

#include "info.h"
#include "str_wrappers.h"


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

s = (char **) nullptr;

Name = (char *) nullptr;

LowerCaseName = (char *) nullptr;

Scope = (char *) nullptr;

U_Scope = (char *) nullptr;

Header = (char *) nullptr;

Nalloc = Nids = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::clear()

{

if ( !s )  return;


int j;

for (j=0; j<Nids; ++j)  {

   if ( s[j] )  { delete [] s[j];  s[j] = (char *) nullptr; }

}

delete [] s;   s = (char **) nullptr;

if ( Name )  { delete [] Name;  Name = (char *) nullptr; }

if ( LowerCaseName )  { delete [] LowerCaseName;  LowerCaseName = (char *) nullptr; }

if ( Scope )  { delete [] Scope;  Scope = (char *) nullptr; }

if ( U_Scope )  { delete [] U_Scope;  U_Scope = (char *) nullptr; }

if ( Header )  { delete [] Header;  Header = (char *) nullptr; }

Nalloc = Nids = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::assign(const EnumInfo & e)

{

clear();

if ( !(e.s) )  return;

int j;

extend(e.Nids);

for (j=0; j<(e.Nids); ++j)  {

   add_id(e.s[j]);

}


set_name(e.Name);

set_header(e.Header);

set_scope(e.Scope);


return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::extend(int n)

{

if ( n < Nalloc )  return;

n = (n + enuminfo_alloc_increment - 1)/enuminfo_alloc_increment;

n *= enuminfo_alloc_increment;

int j;
char ** u = (char **) nullptr;

u = s;

s = new char * [n];

if ( !s )  {

   cerr << "\n\n  EnumInfo::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for (j=0; j<n; ++j)  s[j] = (char *) nullptr;

if ( u )  {

   for (j=0; j<Nids; ++j)  {

      s[j] = u[j];

   }

}

Nalloc = n;

return;

}


////////////////////////////////////////////////////////////////////////


const char * EnumInfo::id(int n) const

{

if ( (n < 0) || (n >= Nids) )  {

   cerr << "\n\n  EnumInfo::id(int) -> range check error\n\n";

   exit ( 1 );

}

return s[n];

}


////////////////////////////////////////////////////////////////////////


int EnumInfo::max_id_length() const

{

if ( Nids == 0 )  return 0;

int j, k;
int max_len;

max_len = 0;

for (j=0; j<Nids; ++j)  {

   k = m_strlen(s[j]);

   if ( k > max_len )  max_len = k;

}


return ( max_len );

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::add_id(const char * text)

{

int k;
const char *method_name = "EnumInfo::add_id() -> ";

extend(Nids + 1);

k = m_strlen(text);

s[Nids] = new char [1 + k];

m_strncpy(s[Nids], text, k, method_name);

s[Nids][k] = (char) 0;   //  just to make sure

++Nids;

return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::set_name(const char * text)

{
const char *method_name = "EnumInfo::set_name() -> ";

if ( Name )  { delete [] Name;  Name = (char *) nullptr; }

if ( LowerCaseName )  { delete [] LowerCaseName;  LowerCaseName = (char *) nullptr; }

if ( !text )  return;

int j, k;

k = m_strlen(text);

Name = new char [1 + k];

LowerCaseName = new char [1 + k];

m_strncpy(Name, text, k, method_name, "Name");

Name[k] = (char) 0;   //  just to make sure

m_strncpy(LowerCaseName, text, k, method_name, "LowerCaseName");

LowerCaseName[k] = (char) 0;   //  just to make sure

for (j=0; j<k; ++j)  {

   LowerCaseName[j] = tolower(LowerCaseName[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::set_scope(const char * text)
{
const char *method_name = "EnumInfo::set_scope() -> ";

if ( Scope )  { delete [] Scope;  Scope = (char *) nullptr; }

if ( U_Scope )  { delete [] U_Scope;  U_Scope = (char *) nullptr; }

if ( !text )  return;

int j, k, m;
char c;


k = m_strlen(text);

Scope = new char [1 + k];

m_strncpy(Scope, text, k, method_name);

Scope[k] = (char) 0;   //  just to make sure

U_Scope = new char [1 + k];

m = 0;

for (j=0; j<k; ++j)  {

   c = Scope[j];

   if ( !c )  break;

   if ( c == ':' )  { ++j;  U_Scope[m++] = '_'; }
   else             U_Scope[m++] = c;

}

U_Scope[m] = (char) 0;


return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::set_header(const char * text)

{
const char *method_name = "EnumInfo::set_header() -> ";

if ( Header )  { delete [] Header;  Header = (char *) nullptr; }

int k;

k = m_strlen(text);

Header = new char [1 + k];

m_strncpy(Header, text, k, method_name);

Header[k] = (char) 0;   //  just to make sure


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


if ( e.Name )  c = e.Name;

s << "enum " << c << " from header file ";

c = "(nul)";

if ( e.Header )  c = e.Header;

s << c << "\n";

s << "There are " << (e.Nids) << " ids\n";

for (j=0; j<(e.Nids); ++j)  {

   s << "    " << j << "   \"";

   s << e.id(j);

   s << "\"\n";

}




s.flush();

return s;

}


////////////////////////////////////////////////////////////////////////




