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
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "afm.h"
#include "afm_line.h"
#include "afm_token.h"

#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const int max_liginfos = 10;

static LigatureInfo liginfo[max_liginfos];

static int n_liginfos = 0;


////////////////////////////////////////////////////////////////////////


static void set_string(char * & s, const char * text);

static void clear_liginfos();


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AfmBBox
   //


////////////////////////////////////////////////////////////////////////


AfmBBox::AfmBBox()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AfmBBox::~AfmBBox()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AfmBBox::AfmBBox(const AfmBBox & b)

{

init_from_scratch();

assign(b);

}


////////////////////////////////////////////////////////////////////////


AfmBBox & AfmBBox::operator=(const AfmBBox & b)

{

if ( this == &b )  return ( * this );

assign(b);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AfmBBox::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AfmBBox::clear()

{

L = B = R = T = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void AfmBBox::assign(const AfmBBox & b)

{

clear();

L = b.L;
R = b.R;

B = b.B;
T = b.T;


return;

}


////////////////////////////////////////////////////////////////////////


void AfmBBox::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "L = " << L << "\n";
out << prefix << "R = " << R << "\n";
out << prefix << "B = " << B << "\n";
out << prefix << "T = " << T << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class LigatureInfo
   //


////////////////////////////////////////////////////////////////////////


LigatureInfo::LigatureInfo()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


LigatureInfo::~LigatureInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


LigatureInfo::LigatureInfo(const LigatureInfo & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


LigatureInfo & LigatureInfo::operator=(const LigatureInfo & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void LigatureInfo::init_from_scratch()

{

successor_name = (char *) 0;

ligature_name = (char *) 0;

clear();



return;

}


////////////////////////////////////////////////////////////////////////


void LigatureInfo::clear()

{

if ( successor_name )  { delete [] successor_name;  successor_name = (char *) 0; }

if ( ligature_name )  { delete [] ligature_name;  ligature_name = (char *) 0; }

successor_index = ligature_index = -1;


return;

}


////////////////////////////////////////////////////////////////////////


void LigatureInfo::assign(const LigatureInfo & i)

{

clear();

successor_index = i.successor_index;
ligature_index = i.ligature_index;

set_string(successor_name, i.successor_name);
set_string(ligature_name, i.ligature_name);


return;

}


////////////////////////////////////////////////////////////////////////


void LigatureInfo::dump(ostream & out, int depth) const

{

Indent prefix(depth);

if ( successor_name )  {

   out << prefix << "successor_name       = \"" << successor_name << "\"\n";

} else {

   out << prefix << "successor_name       = (nul)\n";

}

out << prefix << "successor_index      = " << successor_index << "\n";





if ( ligature_name )  {

   out << prefix << "ligature_name       = \"" << ligature_name << "\"\n";

} else {

   out << prefix << "ligature_name       = (nul)\n";

}

out << prefix << "ligature_index      = " << ligature_index << "\n";


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AfmCharMetrics
   //


////////////////////////////////////////////////////////////////////////


AfmCharMetrics::AfmCharMetrics()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AfmCharMetrics::~AfmCharMetrics()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AfmCharMetrics::AfmCharMetrics(const AfmCharMetrics & m)

{

init_from_scratch();

assign(m);

}


////////////////////////////////////////////////////////////////////////


AfmCharMetrics & AfmCharMetrics::operator=(const AfmCharMetrics & m)

{

if ( this == &m )  return ( * this );

assign(m);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AfmCharMetrics::init_from_scratch()

{

name = (char *) 0;

linfo = (LigatureInfo *) 0;

clear();


return;

}


////////////////////////////////////////////////////////////////////////


void AfmCharMetrics::clear()

{

if ( name )  { delete [] name;  name = (char *) 0; }

if ( linfo )  { delete [] linfo;  linfo = (LigatureInfo *) 0; }


ascii_code = -1;

width = 0;

bbox.clear();

n_ligatures = 0;



return;

}


////////////////////////////////////////////////////////////////////////


void AfmCharMetrics::assign(const AfmCharMetrics & m)

{

clear();


ascii_code = m.ascii_code;

width = m.width;

bbox = m.bbox;

n_ligatures = m.n_ligatures;

set_string(name, m.name);

if ( m.linfo )  {

   linfo = new LigatureInfo [m.n_ligatures];

   int j;

   for (j=0; j<(m.n_ligatures); ++j)  {

      linfo[j] = m.linfo[j];

   }

}



return;

}


////////////////////////////////////////////////////////////////////////


void AfmCharMetrics::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "ascii_code  = " << ascii_code  << "\n";
out << prefix << "width       = " << width       << "\n";
out << prefix << "n_ligatures = " << n_ligatures << "\n";

if ( name )  {

   out << prefix << "name        = \"" << name << "\"\n";

} else {

   out << prefix << "name        = (nul)\n";

}

out << prefix << "bbox ...\n";

bbox.dump(out, depth + 1);

int j;

for (j=0; j<n_ligatures; ++j)  {

   out << prefix << "LigatureInfo # " << j << " ... \n";

   linfo[j].dump(out, depth + 1);

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PCC
   //


////////////////////////////////////////////////////////////////////////


PCC::PCC()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


PCC::~PCC()

{

clear();

}


////////////////////////////////////////////////////////////////////////


PCC::PCC(const PCC & p)

{

init_from_scratch();

assign(p);

}


////////////////////////////////////////////////////////////////////////


PCC & PCC::operator=(const PCC & p)

{

if ( this == &p )  return ( * this );

assign(p);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void PCC::init_from_scratch()

{

name = (char *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void PCC::clear()

{

if ( name )  { delete [] name;  name = (char *) 0; }

delta_x = 0;

delta_y = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void PCC::assign(const PCC & p)

{

clear();

set_string(name, p.name);

delta_x = p.delta_x;

delta_y = p.delta_y;

return;

}


////////////////////////////////////////////////////////////////////////


void PCC::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "name    = \"" << name    << "\"\n";
out << prefix << "delta_x = "   << delta_x << "\n";
out << prefix << "delta_y = "   << delta_y << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AfmCompositeInfo
   //


////////////////////////////////////////////////////////////////////////


AfmCompositeInfo::AfmCompositeInfo()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


AfmCompositeInfo::~AfmCompositeInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


AfmCompositeInfo::AfmCompositeInfo(const AfmCompositeInfo & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


AfmCompositeInfo & AfmCompositeInfo::operator=(const AfmCompositeInfo & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void AfmCompositeInfo::init_from_scratch()

{

name = (char *) 0;

pcc = (PCC *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void AfmCompositeInfo::clear()

{

if ( name )  { delete [] name;  name = (char *) 0; }

if ( pcc )   { delete [] pcc;   pcc  = (PCC *) 0;  }

n_parts = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void AfmCompositeInfo::assign(const AfmCompositeInfo & i)

{

int j;

clear();

set_string(name, i.name);

n_parts = i.n_parts;

pcc = new PCC [n_parts];

for (j=0; j<n_parts; ++j)  {

   pcc[j] = i.pcc[j];

}



return;

}


////////////////////////////////////////////////////////////////////////


void AfmCompositeInfo::dump(ostream & out, int depth) const

{

Indent prefix(depth);


out << prefix << "name    = \"" << name << "\"\n";
out << prefix << "n_parts = " << n_parts << "\n";

int j;

for (j=0; j<n_parts; ++j)  {

   out << prefix << "Part " << j << " ...\n";

   pcc[j].dump(out, depth + 1);

}



   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class KPX
   //


////////////////////////////////////////////////////////////////////////


KPX::KPX()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


KPX::~KPX()

{

clear();

}


////////////////////////////////////////////////////////////////////////


KPX::KPX(const KPX & p)

{

init_from_scratch();

assign(p);

}


////////////////////////////////////////////////////////////////////////


KPX & KPX::operator=(const KPX & p)

{

if ( this == &p )  return ( * this );

assign(p);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void KPX::init_from_scratch()

{

name1 = (char *) 0;

name2 = (char *) 0;


clear();


return;

}


////////////////////////////////////////////////////////////////////////


void KPX::clear()

{

if ( name1 )  { delete [] name1;  name1 = (char *) 0; }
if ( name2 )  { delete [] name2;  name2 = (char *) 0; }

dx = 0.0;

return;

}


////////////////////////////////////////////////////////////////////////


void KPX::assign(const KPX & p)

{

clear();

set_string(name1, p.name1);

set_string(name2, p.name2);

dx = p.dx;

return;

}


////////////////////////////////////////////////////////////////////////


void KPX::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "name1 = \"" << name1 << "\"\n";
out << prefix << "name2 = \"" << name2 << "\"\n";
out << prefix << "dx    = "   << dx    << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Afm
   //


////////////////////////////////////////////////////////////////////////


Afm::Afm()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Afm::~Afm()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Afm::Afm(const Afm & a)

{

init_from_scratch();

assign(a);

}


////////////////////////////////////////////////////////////////////////


Afm & Afm::operator=(const Afm & a)

{

if ( this == &a )  return ( * this );

assign(a);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Afm::init_from_scratch()

{

in = (ifstream *) 0;

cm = (AfmCharMetrics *) 0;


FontName       = (char *) 0;
FullName       = (char *) 0;
FamilyName     = (char *) 0;
Weight         = (char *) 0;
Version        = (char *) 0;
EncodingScheme = (char *) 0;


compinfo = (AfmCompositeInfo *) 0;

kpx = (KPX *) 0;




clear();


return;

}


////////////////////////////////////////////////////////////////////////


void Afm::clear()

{

if ( in )  { delete in;  in = (ifstream *) 0; }

line_number = 0;


if ( FontName       )  { delete [] FontName;        FontName       = (char *) 0; }
if ( FullName       )  { delete [] FullName;        FullName       = (char *) 0; }
if ( FamilyName     )  { delete [] FamilyName;      FamilyName     = (char *) 0; }
if ( Weight         )  { delete [] Weight;          Weight         = (char *) 0; }
if ( Version        )  { delete [] Version;         Version        = (char *) 0; }
if ( EncodingScheme )  { delete [] EncodingScheme;  EncodingScheme = (char *) 0; }


if ( cm )  { delete [] cm;  cm = (AfmCharMetrics *) 0; }

n_cms = 0;

if ( compinfo )  { delete [] compinfo;  compinfo = (AfmCompositeInfo *) 0; }

n_composites = 0;

if ( kpx )  { delete [] kpx;  kpx = (KPX *) 0; }

n_kern_pairs = 0;

ItalicAngle = 0.0;

IsFixedPitch = 0;

UnderlinePosition  = 0.0;
UnderlineThickness = 0.0;

FontBBox.clear();

CapHeight  = 0.0;
XHeight    = 0.0;
Ascender   = 0.0;
Descender  = 0.0;


return;

}


////////////////////////////////////////////////////////////////////////


void Afm::assign(const Afm & a)

{

int j;


clear();

set_string(FontName,       a.FontName      );
set_string(FullName,       a.FullName      );
set_string(FamilyName,     a.FamilyName    );
set_string(Weight,         a.Weight        );
set_string(Version,        a.Version       );
set_string(EncodingScheme, a.EncodingScheme);

ItalicAngle = a.ItalicAngle;

IsFixedPitch = a.IsFixedPitch;

UnderlinePosition = a.UnderlinePosition;

UnderlineThickness = a.UnderlineThickness;

FontBBox = a.FontBBox;

CapHeight = a.CapHeight;

XHeight = a.XHeight;

Ascender = a.Ascender;

Descender = a.Descender;



n_cms = a.n_cms;

if ( n_cms > 0 )  {

   cm = new AfmCharMetrics [n_cms];

   for (j=0; j<n_cms; ++j)  {

      cm[j] = a.cm[j];

   }

}



n_composites = a.n_composites;

if ( n_composites > 0 )  {

   compinfo = new AfmCompositeInfo [n_composites];

   for (j=0; j<n_composites; ++j)  {

      compinfo[j] = a.compinfo[j];

   }

}



n_kern_pairs = a.n_kern_pairs;

if ( n_kern_pairs > 0 )  {

   kpx = new KPX [n_kern_pairs];

   for (j=0; j<n_kern_pairs; ++j)  {

      kpx[j] = a.kpx[j];

   }

}



return;

}


////////////////////////////////////////////////////////////////////////


void Afm::dump(ostream & out, int depth) const

{

int j;
Indent prefix(depth);
Indent prefix2(depth + 1);


if ( FontName       )  out << prefix << "FontName           = \"" << FontName       << "\"\n";
else                   out << prefix << "FontName           = (nul)\n";

if ( FullName       )  out << prefix << "FullName           = \"" << FullName       << "\"\n";
else                   out << prefix << "FullName           = (nul)\n";

if ( FamilyName     )  out << prefix << "FamilyName         = \"" << FamilyName     << "\"\n";
else                   out << prefix << "FamilyName         = (nul)\n";

if ( Weight         )  out << prefix << "Weight             = \"" << Weight         << "\"\n";
else                   out << prefix << "Weight             = (nul)\n";

if ( Version        )  out << prefix << "Version            = \"" << Version        << "\"\n";
else                   out << prefix << "Version            = (nul)\n";

if ( EncodingScheme )  out << prefix << "EncodingScheme     = \"" << EncodingScheme << "\"\n";
else                   out << prefix << "EncodingScheme     = (nul)\n";

out << prefix << "ItalicAngle        = " << ItalicAngle        << "\n";
out << prefix << "IsFixedPitch       = " << IsFixedPitch       << "\n";
out << prefix << "UnderlinePosition  = " << UnderlinePosition  << "\n";
out << prefix << "UnderlineThickness = " << UnderlineThickness << "\n";
out << prefix << "CapHeight          = " << CapHeight          << "\n";
out << prefix << "XHeight            = " << XHeight            << "\n";
out << prefix << "Ascender           = " << Ascender           << "\n";
out << prefix << "Descender          = " << Descender          << "\n";
out << prefix << "n_cms              = " << n_cms              << "\n";
out << prefix << "n_composites       = " << n_composites       << "\n";
out << prefix << "n_kern_pairs       = " << n_kern_pairs       << "\n";

out << prefix << "CharMetrics ... \n";

for (j=0; j<n_cms; ++j)  {

   out << prefix2 << "CharMetric " << j << " of " << n_cms << " ... \n";

   cm[j].dump(out, depth + 2);

}


if ( n_composites > 0 )  {

   out << prefix << "Composites ... \n";

   for (j=0; j<n_composites; ++j)  {

      out << prefix2 << "Composite " << j << " of " << n_composites << " ... \n";

      compinfo[j].dump(out, depth + 2);

   }

}


if ( n_kern_pairs > 0 )  {

   out << prefix << "Kern Pairs ... \n";

   for (j=0; j<n_kern_pairs; ++j)  {

      out << prefix2 << "Kern Pair " << j << " of " << n_kern_pairs << " ... \n";

      kpx[j].dump(out, depth + 2);

   }

}








   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


int Afm::read(const ConcatString& filename)

{

int j, k;
AfmLine line;
AfmToken tok;



clear();

clear_liginfos();



in = new ifstream;

met_open(*in, filename.c_str());
//in->open(filename.c_str());

if ( !(*in) )  {

   mlog << Error << "\nAfm::read() -> unable to open input file \"" << filename << "\"\n\n";

   exit ( 1 );

}


while ( (*in) >> line )  {

   line.line_number = ++line_number;

   tok = line.nexttoken();

   if ( tok.type == afm_token_endofline )  {

      mlog << Debug(1) << "End of Line ... \n";

      tok.dump(cout);

      break;

   }

   if ( tok.type != afm_token_keyword )  {

      mlog << Error << "\nAfm::read() -> expecting keyword!\n\n";

      tok.dump(cerr);

      exit ( 1 );

   }

   switch ( tok.keyword )  {


      case afm_keyword_StartFontMetrics:
         do_startfontmetrics();
         break;


      default:
         mlog << Error << "\nAfm::read() -> bad keyword\n\n";
         tok.dump(cerr);
         exit ( 1 );
         break;

   }   //  switch

}   //  while



   //
   //  patch ligatures
   //

for (j=0; j<n_cms; ++j)  {

   if ( cm[j].n_ligatures == 0 )  continue;

   for (k=0; k<(cm[j].n_ligatures); ++k)  {

      patch_ligatures(cm[j].linfo[k]);

   }

}



   //
   //  done
   //

in->close();

delete in;  in = (ifstream *) 0;

line_number = 0;


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void Afm::do_fontbbox(AfmLine & line)

{

AfmToken tok;

tok = line.nexttoken();
FontBBox.L = tok.i;

tok = line.nexttoken();
FontBBox.B = tok.i;

tok = line.nexttoken();
FontBBox.R = tok.i;

tok = line.nexttoken();
FontBBox.T = tok.i;



return;

}


////////////////////////////////////////////////////////////////////////


void Afm::do_startfontmetrics()

{

AfmLine line;
AfmToken tok;


while ( (*in) >> line )  {

   line.line_number = ++line_number;

   tok = line.nexttoken();

   if ( tok.type != afm_token_keyword )  {

      mlog << Error << "\nAfm::do_startfontmetrics() -> expecting keyword!\n\n";

      tok.dump(cerr);

      exit ( 1 );

   }

   switch ( tok.keyword )  {

      case afm_keyword_EndFontMetrics:
      case afm_keyword_Comment:
      case afm_keyword_Notice:
         break;

      case afm_keyword_StartCharMetrics:
         tok = line.nexttoken();
         n_cms = tok.i;
         cm = new AfmCharMetrics [n_cms];
         do_startcharmetrics();
         break;

      case afm_keyword_StartComposites:
         tok = line.nexttoken();
         n_composites = tok.i;
         compinfo = new AfmCompositeInfo [n_composites];
         do_startcomposites();
         break;

      case afm_keyword_StartKernData:
         do_startkerndata();
         break;


      case afm_keyword_FontName:
         tok = line.rest_as_string();
         set_string(FontName, tok.s.c_str());
         break;

      case afm_keyword_FullName:
         tok = line.rest_as_string();
         set_string(FullName, tok.s.c_str());
         break;

      case afm_keyword_FamilyName:
         tok = line.rest_as_string();
         set_string(FamilyName, tok.s.c_str());
         break;

      case afm_keyword_Weight:
         tok = line.rest_as_string();
         set_string(Weight, tok.s.c_str());
         break;

      case afm_keyword_Version:
         tok = line.rest_as_string();
         set_string(Version, tok.s.c_str());
         break;

      case afm_keyword_EncodingScheme:
         tok = line.rest_as_string();
         set_string(EncodingScheme, tok.s.c_str());
         break;

      case afm_keyword_ItalicAngle:
         tok = line.nexttoken();
         ItalicAngle = atof(tok.s.c_str());
         break;

      case afm_keyword_IsFixedPitch:
         tok = line.nexttoken();
         IsFixedPitch = tok.i;
         break;

      case afm_keyword_UnderlinePosition:
         tok = line.nexttoken();
         UnderlinePosition = tok.as_double();
         break;

      case afm_keyword_UnderlineThickness:
         tok = line.nexttoken();
         UnderlineThickness = tok.as_double();
         break;

      case afm_keyword_FontBBox:
         do_fontbbox(line);
         break;

      case afm_keyword_CapHeight:
         tok = line.nexttoken();
         CapHeight = tok.as_double();
         break;

      case afm_keyword_XHeight:
         tok = line.nexttoken();
         XHeight = tok.as_double();
         break;

      case afm_keyword_Ascender:
         tok = line.nexttoken();
         Ascender = tok.as_double();
         break;

      case afm_keyword_Descender:
         tok = line.nexttoken();
         Descender = tok.as_double();
         break;


      default:
         mlog << Error << "\nAfm::do_startfontmetrics() -> bad keyword\n\n";
         tok.dump(cerr);
         exit ( 1 );
         break;

   }   //  switch

   if ( tok.keyword  == afm_keyword_EndFontMetrics )  break;

}   //  while






return;

}


////////////////////////////////////////////////////////////////////////


void Afm::do_startcharmetrics()

{

AfmLine line;
AfmToken tok;
int n;


n = 0;

while ( (*in) >> line )  {

   line.line_number = ++line_number;

   tok = line.nexttoken();

   if ( tok.type != afm_token_keyword )  {

      mlog << Error << "\nAfm::do_startcharmetrics() -> expecting keyword!\n\n";

      tok.dump(cerr);

      exit ( 1 );

   }

   switch ( tok.keyword )  {

      case afm_keyword_EndCharMetrics:
         break;

      case afm_keyword_C:
         if ( n > n_cms )  {
            mlog << Error << "\nAfm::do_startcharmetrics() -> too many char metrics!\n\n";
            exit ( 1 );
         }
         do_c(line, n);
         ++n;
         break;


      default:
         mlog << Error << "\nAfm::do_startcharmetrics() -> bad keyword\n\n";
         tok.dump(cerr);
         exit ( 1 );
         break;

   }   //  switch


   if ( tok.keyword == afm_keyword_EndCharMetrics )  break;


}   //  while






return;

}


////////////////////////////////////////////////////////////////////////


void Afm::do_startkerndata()

{

AfmLine line;
AfmToken tok;



while ( (*in) >> line )  {

   line.line_number = ++line_number;

   tok = line.nexttoken();

   if ( tok.type != afm_token_keyword )  {

      mlog << Error << "\nAfm::do_startkerndata() -> expecting keyword!\n\n";

      tok.dump(cerr);

      exit ( 1 );

   }

   switch ( tok.keyword )  {

      case afm_keyword_StartKernPairs:
         tok = line.nexttoken();
         n_kern_pairs = tok.i;
         kpx = new KPX [n_kern_pairs];
         do_startkernpairs();
         break;

      case afm_keyword_EndKernData:
         break;


      default:
         mlog << Error << "\nAfm::do_startkerndata() -> bad keyword\n\n";
         tok.dump(cerr);
         exit ( 1 );
         break;

   }   //  switch


   if ( tok.keyword == afm_keyword_EndKernData )  break;


}   //  while






return;

}


////////////////////////////////////////////////////////////////////////


void Afm::do_startkernpairs()

{

AfmLine line;
AfmToken tok;
int n;


n = 0;


while ( (*in) >> line )  {

   line.line_number = ++line_number;

   tok = line.nexttoken();

   if ( tok.type != afm_token_keyword )  {

      mlog << Error << "\nAfm::do_startkernpairs() -> expecting keyword!\n\n";

      tok.dump(cerr);

      exit ( 1 );

   }

   switch ( tok.keyword )  {

      case afm_keyword_KPX:
         if ( n > n_kern_pairs )  {
            mlog << Error << "\nAfm::do_startkernpairs() -> too many kern pairs!\n\n";
            exit ( 1 );
         }
         tok = line.nexttoken();
         set_string(kpx[n].name1, tok.s.c_str());
         tok = line.nexttoken();
         set_string(kpx[n].name2, tok.s.c_str());
         tok = line.nexttoken();
         kpx[n].dx = tok.as_double();
         ++n;
         break;

      case afm_keyword_EndKernPairs:
         break;


      default:
         mlog << Error << "\nAfm::do_startkernpairs() -> bad keyword\n\n";
         tok.dump(cerr);
         exit ( 1 );
         break;

   }   //  switch


   if ( tok.keyword == afm_keyword_EndKernPairs )  break;


}   //  while








return;

}


////////////////////////////////////////////////////////////////////////


void Afm::do_startcomposites()

{

AfmLine line;
AfmToken tok;
int n;


n = 0;


while ( (*in) >> line )  {

   line.line_number = ++line_number;

   tok = line.nexttoken();

   if ( tok.type != afm_token_keyword )  {

      mlog << Error << "\nAfm::do_startcomposites() -> expecting keyword!\n\n";

      tok.dump(cerr);

      exit ( 1 );

   }


   switch ( tok.keyword )  {

      case afm_keyword_EndComposites:
         break;

      case afm_keyword_CC:
         if ( n > n_composites )  {
            mlog << Error << "\nAfm::do_startcomposites() -> bad composite count\n\n";
            exit ( 1 );
         }
         do_cc(line, n);
         ++n;
         break;


      default:
         mlog << Error << "\nAfm::do_startcomposites() -> bad keyword\n\n";
         tok.dump(cerr);
         exit ( 1 );
         break;

   }   //  switch

   if ( tok.keyword == afm_keyword_EndComposites )  break;

}   //  while






return;

}


////////////////////////////////////////////////////////////////////////


void Afm::do_c(AfmLine & line, const int n)

{

int j;
AfmToken tok;
int ascii_code;
AfmCharMetrics & c = cm[n];


clear_liginfos();

tok = line.nexttoken();

ascii_code = tok.i;



c.ascii_code = ascii_code;


while ( 1 )  {
   tok = line.nexttoken();

   if ( tok.type == afm_token_endofline )  break;

   if ( tok.type != afm_token_keyword )  {

      mlog << Error << "\nAfm::do_c(AfmLine &) -> bad token (1)\n\n";

      tok.dump(cerr);

      exit ( 1 );

   }

   switch ( tok.keyword )  {

      case afm_keyword_WX:
         tok = line.nexttoken();
         c.width = tok.i;
         break;

      case afm_keyword_N:
         tok = line.nexttoken();
         set_string(c.name, tok.s.c_str());
         break;

      case afm_keyword_B:
         tok = line.nexttoken();
         c.bbox.L = tok.i;
         tok = line.nexttoken();
         c.bbox.B = tok.i;
         tok = line.nexttoken();
         c.bbox.R = tok.i;
         tok = line.nexttoken();
         c.bbox.T = tok.i;
         break;

      case afm_keyword_L:
         if ( n_liginfos >= max_liginfos )  {
            mlog << Error << "\nAfm::do_c(AfmLine &) -> too many ligatures for char \""
                 << (c.name) << "\"\n\n";
            exit ( 1 );
         }
         tok = line.nexttoken();
         set_string(liginfo[n_liginfos].successor_name, tok.s.c_str());
         tok = line.nexttoken();
         set_string(liginfo[n_liginfos].ligature_name, tok.s.c_str());
         ++n_liginfos;
         break;


      default:
         mlog << Error << "\nAfm::do_c(AfmLine &) -> bad token (2)\n\n";
         tok.dump(cerr);
         exit ( 1 );
         break;


   }   //  switch


}   //  while


   //
   //  add ligatures, if needed
   //

if ( n_liginfos > 0 )  {

   c.n_ligatures = n_liginfos;

   c.linfo = new LigatureInfo [n_liginfos];

   for (j=0; j<n_liginfos; ++j)  {

      c.linfo[j] = liginfo[j];

   }

   clear_liginfos();

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Afm::do_cc(AfmLine & line, const int n)

{

int n_pcc;
AfmToken tok;
AfmCompositeInfo & c = compinfo[n];

   //
   //  get name and n_parts
   //

tok = line.nexttoken();

set_string(c.name, tok.s.c_str());

tok = line.nexttoken();

c.n_parts = tok.i;

c.pcc = new PCC [c.n_parts];

n_pcc = 0;

while ( 1 )  {

   tok = line.nexttoken();

   if ( tok.type == afm_token_endofline )  break;

   if ( tok.type != afm_token_keyword )  {

      mlog << Error << "\nAfm::do_cc(AfmLine &) -> bad token (1)\n\n";

      tok.dump(cerr);

      exit ( 1 );

   }

   switch ( tok.keyword )  {

      case afm_keyword_PCC:
         if ( n_pcc > c.n_parts )  {
            mlog << Error << "\nAfm::do_cc(AfmLine &) -> too many composite parts!\n\n";
            exit ( 1 );
         }
         tok = line.nexttoken();
         set_string(c.pcc[n_pcc].name, tok.s.c_str());
         tok = line.nexttoken();
         c.pcc[n_pcc].delta_x = tok.i;
         tok = line.nexttoken();
         c.pcc[n_pcc].delta_y = tok.i;
         ++n_pcc;
         break;



      default:
         mlog << Error << "\nAfm::do_c(AfmLine &) -> bad token (2)\n\n";
         tok.dump(cerr);
         exit ( 1 );
         break;


   }   //  switch


}   //  while







return;

}


////////////////////////////////////////////////////////////////////////


void Afm::patch_ligatures(LigatureInfo & i)

{

int j;
int successor_found = 0;
int ligature_found = 0;


for (j=0; j<n_cms; ++j)  {


   if ( !successor_found && strcmp(i.successor_name, cm[j].name) == 0 )  {

      successor_found = 1;

      i.successor_index = j;

   }


   if ( !ligature_found && strcmp(i.ligature_name, cm[j].name) == 0 )  {

      ligature_found = 1;

      i.ligature_index = j;

   }


}





return;

}


////////////////////////////////////////////////////////////////////////


int Afm::lookup_cm(int ascii_code) const

{

if ( ascii_code < 0 )  ascii_code += 256;   //  in case we're passed a signed char
                                            //    that was promoted to an int

int j;


for (j=0; j<n_cms; ++j)  {

   if ( cm[j].ascii_code == ascii_code )  return ( j );

}



return ( -1 );

}


////////////////////////////////////////////////////////////////////////


int Afm::lookup_cm(const char * name) const

{

int j;


for (j=0; j<n_cms; ++j)  {

   if ( strcmp(cm[j].name, name) == 0 )  return ( j );

}



return ( -1 );

}


////////////////////////////////////////////////////////////////////////


int Afm::has_ligature(int ascii_code_1, int ascii_code_2, LigatureInfo & lig) const

{

lig.clear();

if ( ascii_code_1 < 0 )  ascii_code_1 += 256;
if ( ascii_code_2 < 0 )  ascii_code_2 += 256;

int cm_index;

cm_index = lookup_cm(ascii_code_1);

if ( cm_index < 0 )  return ( 0 );

AfmCharMetrics & m = cm[cm_index];

if ( m.n_ligatures == 0 )  return ( 0 );

int j, k;


for (j=0; j<(m.n_ligatures); ++j)  {

   k = m.linfo[j].successor_index;

   if ( cm[k].ascii_code == ascii_code_2 )  {

      lig = m.linfo[j];

      return ( 1 );

   }

}

   //
   //  nope
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int Afm::has_kern_pair(int ascii_code_1, int ascii_code_2, KPX & kp) const

{

kp.clear();

if ( n_kern_pairs == 0 )  return ( 0 );

if ( ascii_code_1 < 0 )  ascii_code_1 += 256;
if ( ascii_code_2 < 0 )  ascii_code_2 += 256;

int j;
int cm_index_1, cm_index_2;
const char * n1 = (const char *) 0;
const char * n2 = (const char *) 0;

cm_index_1 = lookup_cm(ascii_code_1);
cm_index_2 = lookup_cm(ascii_code_2);

if ( (cm_index_1 < 0) || (cm_index_2 < 0) )  return ( 0 );

n1 = cm[cm_index_1].name;
n2 = cm[cm_index_2].name;


for (j=0; j<n_kern_pairs; ++j)  {

   if ( (strcmp(kpx[j].name1, n1) == 0) && (strcmp(kpx[j].name2, n2) == 0) )  {

      kp = kpx[j];

      return ( 1 );

   }

}

   //
   //  nope
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void set_string(char * & s, const char * text)

{

if ( s )  { delete [] s;  s = (char *) 0; }

if ( !text )  return;

s = new char [1 + strlen(text)];

strcpy(s, text);


return;

}


////////////////////////////////////////////////////////////////////////


void clear_liginfos()

{

int j;


for (j=0; j<max_liginfos; ++j)  {

   liginfo[j].clear();

}


n_liginfos = 0;


return;

}


////////////////////////////////////////////////////////////////////////





