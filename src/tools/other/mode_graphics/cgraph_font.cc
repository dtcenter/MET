// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "cgraph_font.h"


////////////////////////////////////////////////////////////////////////


static const int alloc_inc = 30;

static const int total_predef_fonts = 34;

static const char cg_font_env [] = "MET_FONT_DIR";


////////////////////////////////////////////////////////////////////////


static bool same_font(const CgFont &, const CgFont &);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CgFont
   //


////////////////////////////////////////////////////////////////////////


CgFont::CgFont()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CgFont::~CgFont()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CgFont::CgFont(const CgFont & p)

{

init_from_scratch();

assign(p);

}


////////////////////////////////////////////////////////////////////////


CgFont & CgFont::operator=(const CgFont & p)

{

if ( this == &p )  return ( * this );

assign(p);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CgFont::init_from_scratch()

{

afm = (Afm *) 0;

face = 0;

if ( !get_env(cg_font_env, gs_font_dir) )  {

   mlog << Error
        << "\nCgFont::init_from_scratch() -> "
        << "unable to get environment variable \""
        << cg_font_env << "\"\n\n";

   exit ( 1 );

}

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void CgFont::clear()

{

full_pfb_name.clear();
short_pfb_name.clear();

full_afm_name.clear();
short_afm_name.clear();

ps_name.clear();

orig_ps_size = scaled_ps_size = 0.0;

if ( afm )  { delete afm;  afm = (Afm *) 0; }

ps_font_number = -1;


return;

}


////////////////////////////////////////////////////////////////////////


void CgFont::assign(const CgFont & p)

{

clear();

full_pfb_name  = p.full_pfb_name;
short_pfb_name = p.short_pfb_name;

full_afm_name  = p.full_afm_name;
short_afm_name = p.short_afm_name;

face = p.face;

ps_name = p.ps_name;

if ( p.afm )  {

   afm = new Afm;

   *afm = *(p.afm);

}

ps_font_number = p.ps_font_number;

orig_ps_size = p.orig_ps_size;
scaled_ps_size = p.scaled_ps_size;

return;

}


////////////////////////////////////////////////////////////////////////


void CgFont::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "full_pfb_name  = ";

if ( full_pfb_name.length() == 0 )  out << "(nul)\n";
else                                out << '\"' << full_pfb_name << "\"\n";


out << prefix << "short_pfb_name = ";

if ( short_pfb_name.length() == 0 )  out << "(nul)\n";
else                                 out << '\"' << short_pfb_name << "\"\n";


out << prefix << "full_afm_name  = ";

if ( full_afm_name.length() == 0 )  out << "(nul)\n";
else                                out << '\"' << full_afm_name << "\"\n";


out << prefix << "short_afm_name = ";

if ( short_afm_name.length() == 0 )  out << "(nul)\n";
else                                 out << '\"' << short_afm_name << "\"\n";


out << prefix << "ps_name        = ";

if ( ps_name.length() == 0 )  out << "(nul)\n";
else                          out << '\"' << ps_name << "\"\n";


out << prefix << "ps_font_number = " << ps_font_number << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void CgFont::set_by_number(int n)

{

clear();

if ( (n < 0) || (n >= total_predef_fonts) )  {

   mlog << Error << "\nCgFont::set_by_number(int) -> "
        << "range check error\n\n";

   exit ( 1 );

}

char junk[256];

snprintf(junk, sizeof(junk), "%02d.afm", n);

short_afm_name = junk;

full_afm_name << gs_font_dir << '/' << gs_font_dir << '/' << short_afm_name;

if ( !file_exists(full_afm_name.c_str()) )  {

   mlog << Error << "\nCgFont::set_by_number(int) -> "
        << "can't find afm file \"" << full_afm_name << "\"\n\n";

   exit ( 1 );

}

afm = new Afm;

if ( !(afm->read(full_afm_name)) )  {

   mlog << Error << "\nCgFont::set_by_number(int) -> "
        << "trouble reading afm file \"" << full_afm_name << "\"\n\n";

   exit ( 1 );

}

ps_name = afm->FontName;

ps_font_number = n;



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CgFontCollection
   //


////////////////////////////////////////////////////////////////////////


CgFontCollection::CgFontCollection()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CgFontCollection::~CgFontCollection()

{

clear();

}


////////////////////////////////////////////////////////////////////////


CgFontCollection::CgFontCollection(const CgFontCollection & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


CgFontCollection & CgFontCollection::operator=(const CgFontCollection & c)

{

if ( this == &c )  return ( * this );

assign(c);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CgFontCollection::init_from_scratch()

{

Nelements = Nalloc = 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void CgFontCollection::clear()

{

int j, error;

for (j=0; j<Nelements; ++j)  {

   error = FT_Done_Face(e[j].face);

   if ( error )  {

      mlog << Error << "\nCgFontCollection::clear() -> "
           << "trouble closing typeface \"" << e[j].short_pfb_name
           << "\"\n\n";

      exit ( 1 );

   }

   e[j].face = 0;

}

e.clear();

Nelements = Nalloc = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void CgFontCollection::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);


out << prefix << "Nelements = " << Nelements << "\n";
out << prefix << "Nalloc    = " << Nalloc    << "\n";

for (j=0; j<Nelements; ++j)  {

   out << prefix << "\n";

   out << prefix << "Element # " << j << " ...\n";

   e[j].dump(out, depth + 1);

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void CgFontCollection::extend(int n)

{

if ( n <= Nalloc )  return;

int k = n/alloc_inc;

if ( n%alloc_inc )  ++k;

n = k*alloc_inc;

e.reserve(n);

Nalloc = n;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void CgFontCollection::assign(const CgFontCollection & c)

{

clear();

if ( c.e.size() == 0 )  return;

extend(c.Nelements);

e = c.e;

Nelements = c.Nelements;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void CgFontCollection::add(const CgFont & f)

{

extend(Nelements + 1);

e.push_back(f);

++Nelements;


return;

}


////////////////////////////////////////////////////////////////////////


void CgFontCollection::add_no_repeat(const CgFont & f)

{

if ( have_it(f) )  return;

extend(Nelements + 1);

e.push_back(f);

++Nelements;


return;

}


////////////////////////////////////////////////////////////////////////


bool CgFontCollection::have_it(const CgFont & f) const

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( same_font( e[j], f ) )  return ( true );

}

return ( false );

}


////////////////////////////////////////////////////////////////////////


CgFont * CgFontCollection::lookup_by_ps_font_number(int n)

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( e[j].ps_font_number == n )  return ( & e[j] );

}


return ( (CgFont *) 0 );

}


////////////////////////////////////////////////////////////////////////


CgFont * CgFontCollection::lookup_by_ps_name(const char * name)

{

int j;

for (j=0; j<Nelements; ++j)  {

   if ( e[j].ps_name == name )  return ( & e[j] );

}


return ( (CgFont *) 0 );

}


////////////////////////////////////////////////////////////////////////


CgFont * CgFontCollection::operator[](int k)

{

if ( (k < 0) || (k >= Nelements) )  {

   mlog << Error << "\nCgFont * CgFontCollection::operator[](int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}

return ( & e[k] );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


bool same_font(const CgFont & a, const CgFont & b)

{

if (  a.ps_name == b.ps_name )  return ( true );

return ( false );

}


////////////////////////////////////////////////////////////////////////
