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
#include <cmath>

#include "pxm_base.h"

#include "vx_log.h"
#include "check_endian.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PxmBase
   //


////////////////////////////////////////////////////////////////////////


PxmBase::PxmBase()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


PxmBase::~PxmBase()

{

clear_common();

}


////////////////////////////////////////////////////////////////////////


void PxmBase::init_from_scratch()

{


Name.clear();


clear_common();


return;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::clear_common()

{

data.clear();

Nalloc = 0;

Name.clear();

Nrows = Ncols = 0;

Ncomments = 0;

   //
   //  a loop, not memset: Comment holds std::string now
   //

for (int j=0; j<max_comments; ++j)  Comment[j].clear();



return;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::copy_common(const PxmBase & p)

{
const char *method_name = "PxmBase::copy_common() ";

if ( this == &p )  return;

clear_common();

Nalloc = p.Nalloc;

data   = p.data;

Nrows = p.Nrows;
Ncols = p.Ncols;

Name = p.Name;

if ( p.Ncomments > 0 )  {

   int j;
   char a_var_name[512+1];

   Ncomments = p.Ncomments;

   for (j=0; j<Ncomments; ++j)  {

      Comment[j] = p.Comment[j];

   }

}



return;

}


////////////////////////////////////////////////////////////////////////


int PxmBase::rc_to_n(int r, int c) const

{

if ( (r < 0) || (r >= Nrows) || (c < 0) || (c >= Ncols) )  {

   mlog << Error << "\nPxmBase::rc_to_n() -> range check error\n\n";

   exit ( 1 );

}

int n;

n = r*Ncols + c;


return n;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::n_to_rc(int n, int & r, int & c) const

{

if ( (n < 0) || (n >= (Nrows*Ncols)) )  {

   mlog << Error << "\nPxmBase::n_to_rc() -> range check error\n\n";

   exit ( 1 );

}

c = n%Ncols;

r = n/Ncols;

return;

}


////////////////////////////////////////////////////////////////////////


const char * PxmBase::short_name() const

{

if ( Name.empty() )  return (const char *) nullptr;

const size_t slash = Name.find_last_of('/');

return ( slash == std::string::npos ? Name.c_str() : Name.c_str() + slash + 1 );

}


////////////////////////////////////////////////////////////////////////


const char * PxmBase::comment(int n) const

{

if ( (n < 0) || (n >= Ncomments) )  {

   mlog << Error << "\nPxmBase::comment(int) const -> range check error!\n\n";

   exit ( 1 );

}


return Comment[n].c_str();

}


////////////////////////////////////////////////////////////////////////


void PxmBase::add_comment(const char * text)

{

const char *method_name = "PxmBase::add_comment(const char *) -> ";

if ( Ncomments >= max_comments )  {

   mlog << Error << "\n" << method_name << "too meny comments!\n\n";

   exit ( 1 );

}

Comment[Ncomments] = text;

++Ncomments;


return;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::clear_comments()

{

int j;

for (j=0; j<max_comments; ++j)  {

   Comment[j].clear();

}

Ncomments = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::dump(ostream & out, int depth) const

{

int j;
unsigned long u;
Indent prefix(depth);
Indent prefix2(depth + 1);


out << prefix << "Name      = ";

if ( !Name.empty() )  out << "\"" << Name << "\"\n";
else         out << "(nul)\n";

out << prefix << "data      = ";

if ( !(data.empty()) )  {

   u = (unsigned long) data.data();

   out << u << "\n";

} else {

   out << "(nul)\n";

}


out << prefix << "Nalloc    = " << Nalloc    << "\n";
out << prefix << "Nrows     = " << Nrows     << "\n";
out << prefix << "Ncols     = " << Ncols     << "\n";
out << prefix << "Ncomments = " << Ncomments << "\n";


for (j=0; j<Ncomments; ++j)  {

   mlog << Debug(1) << prefix2
        << "Comment[" << j << "] = \""
        << Comment[j] << "\"\n";

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::copy_data(unsigned char * out) const

{

const int n = n_data_bytes();

memcpy(out, data.data(), n);


return;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::copy_data_32(unsigned char * out, const bool swap_endian) const

{

int j;
const int nxy = Nrows*Ncols;
unsigned char * u = out;
unsigned int  * i = (unsigned int *) out;
const unsigned char * d = data.data();

j = 0;

while ( j < nxy )  {

   *u++ = 0;

   *u++ = *d++;
   *u++ = *d++;
   *u++ = *d++;

   // *u++ = 0;

   if ( swap_endian )  shuffle_4(i);

   ++j;

   ++i;

}   //  while


return;

}



////////////////////////////////////////////////////////////////////////






