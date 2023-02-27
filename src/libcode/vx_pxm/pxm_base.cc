// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

#include "pxm_base.h"

#include "vx_log.h"
#include "check_endian.h"


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

data = (unsigned char *) 0;

Name = (char *) 0;


clear_common();


return;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::clear_common()

{

if ( data )  { delete [] data;  data = (unsigned char *) 0; }

Nalloc = 0;

if ( Name )  { delete [] Name;  Name = (char *) 0; }

Nrows = Ncols = 0;

Ncomments = 0;

memset(Comment, 0, sizeof(Comment));



return;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::copy_common(const PxmBase & p)

{
const char *method_name = "PxmBase::copy_common() ";

if ( this == &p )  return;

clear_common();

if ( p.data )  {

   Nalloc = p.Nalloc;

   data = new unsigned char [Nalloc];

   memcpy(data, p.data, Nalloc);

}

Nrows = p.Nrows;
Ncols = p.Ncols;

if ( p.Name )  {

   Name = m_strcpy2(p.Name, method_name, "Name");

}

if ( p.Ncomments > 0 )  {

   int j;
   char a_var_name[512+1];

   Ncomments = p.Ncomments;

   for (j=0; j<Ncomments; ++j)  {

      snprintf(a_var_name, 512, "Comment[%d]", j);
      Comment[j] = m_strcpy2(p.Comment[j], method_name, a_var_name);

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


return ( n );

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

if ( !Name )  return ( (const char *) 0 );

int j;

j = m_strlen(Name) - 1;

while ( (j >= 0) && (Name[j] != '/') )  --j;

++j;


return ( Name + j );

}


////////////////////////////////////////////////////////////////////////


const char * PxmBase::comment(int n) const

{

if ( (n < 0) || (n >= Ncomments) )  {

   mlog << Error << "\nPxmBase::comment(int) const -> range check error!\n\n";

   exit ( 1 );

}


return ( Comment[n] );

}


////////////////////////////////////////////////////////////////////////


void PxmBase::add_comment(const char * text)

{

const char *method_name = "PxmBase::add_comment(const char *) -> ";

if ( Ncomments >= max_comments )  {

   mlog << Error << "\n" << method_name << "too meny comments!\n\n";

   exit ( 1 );

}

Comment[Ncomments] = m_strcpy2(text, method_name, "Comment");

++Ncomments;


return;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::clear_comments()

{

int j;

for (j=0; j<max_comments; ++j)  {

   if ( Comment[j] )  {

      delete [] Comment[j];  Comment[j] = (char *) 0;

   }

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

if ( Name )  out << "\"" << Name << "\"\n";
else         out << "(nul)\n";

out << prefix << "data      = ";

if ( data )  {

   u = (unsigned long) data;

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

memcpy(out, data, n);


return;

}


////////////////////////////////////////////////////////////////////////


void PxmBase::copy_data_32(unsigned char * out, const bool swap_endian) const

{

int j;
const int nxy = Nrows*Ncols;
unsigned char * u = out;
unsigned int  * i = (unsigned int *) out;
unsigned char * d = data;

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






