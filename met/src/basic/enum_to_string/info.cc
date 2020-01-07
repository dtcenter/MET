// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

if ( this == &e )  return ( * this );

assign(e);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::init_from_scratch()

{

s = (char **) 0;

Name = (char *) 0;

LowerCaseName = (char *) 0;

Scope = (char *) 0;

U_Scope = (char *) 0;

Header = (char *) 0;

Nalloc = Nids = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::clear()

{

if ( !s )  return;


int j;

for (j=0; j<Nids; ++j)  {

   if ( s[j] )  { delete [] s[j];  s[j] = (char *) 0; }

}

delete [] s;   s = (char **) 0;

if ( Name )  { delete [] Name;  Name = (char *) 0; }

if ( LowerCaseName )  { delete [] LowerCaseName;  LowerCaseName = (char *) 0; }

if ( Scope )  { delete [] Scope;  Scope = (char *) 0; }

if ( U_Scope )  { delete [] U_Scope;  U_Scope = (char *) 0; }

if ( Header )  { delete [] Header;  Header = (char *) 0; }

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
char ** u = (char **) 0;

u = s;

s = new char * [n];

if ( !s )  {

   cerr << "\n\n  EnumInfo::extend(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for (j=0; j<n; ++j)  s[j] = (char *) 0;

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

return ( s[n] );

}


////////////////////////////////////////////////////////////////////////


int EnumInfo::max_id_length() const

{

if ( Nids == 0 )  return ( 0 );

int j, k;
int max_len;

max_len = 0;

for (j=0; j<Nids; ++j)  {

   k = strlen(s[j]);

   if ( k > max_len )  max_len = k;

}


return ( max_len );

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::add_id(const char * text)

{

int k;

extend(Nids + 1);

k = strlen(text);

s[Nids] = new char [1 + k];

strncpy(s[Nids], text, k);

s[Nids][k] = (char) 0;   //  just to make sure

++Nids;

return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::set_name(const char * text)

{

if ( Name )  { delete [] Name;  Name = (char *) 0; }

if ( LowerCaseName )  { delete [] LowerCaseName;  LowerCaseName = (char *) 0; }

if ( !text )  return;

int j, k;

k = strlen(text);

Name = new char [1 + k];

LowerCaseName = new char [1 + k];

strncpy(Name, text, k);

Name[k] = (char) 0;   //  just to make sure

strncpy(LowerCaseName, text, k);

LowerCaseName[k] = (char) 0;   //  just to make sure

for (j=0; j<k; ++j)  {

   LowerCaseName[j] = tolower(LowerCaseName[j]);

}

return;

}


////////////////////////////////////////////////////////////////////////


void EnumInfo::set_scope(const char * text)

{

if ( Scope )  { delete [] Scope;  Scope = (char *) 0; }

if ( U_Scope )  { delete [] U_Scope;  U_Scope = (char *) 0; }

if ( !text )  return;

int j, k, m;
char c;


k = strlen(text);

Scope = new char [1 + k];

strncpy(Scope, text, k);

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

if ( Header )  { delete [] Header;  Header = (char *) 0; }

int k;

k = strlen(text);

Header = new char [1 + k];

strncpy(Header, text, k);

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

return ( s );

}


////////////////////////////////////////////////////////////////////////




