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
#include <fstream>
#include <unistd.h>
#include <cstdio>
#include <sys/file.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <memory.h>
#include <cmath>

#include "vx_log.h"
#include "vx_util.h"
#include "vx_ps.h"
#include "documentmedia_to_string.h"
#include "documentorientation_to_string.h"
#include "fontfamily_to_string.h"

#include "flate_filter.h"
#include "ascii85_filter.h"
#include "psout_filter.h"


////////////////////////////////////////////////////////////////////////

static const int ml_prec = 1;   //  moveto/lineto precision
// static const int ml_prec = 5;   //  moveto/lineto precision

static const DocumentOrientation default_orientation = OrientationPortrait;

static const FontFamily default_font_family    = ff_Times;

   //
   //  paper sizes
   //

static const double Letter_pagewidth  =  8.5*72.0;
static const double Letter_pageheight = 11.0*72.0;

static const double A4_pagewidth      =  8.27*72.0;
static const double A4_pageheight     = 11.69*72.0;


////////////////////////////////////////////////////////////////////////


static void base_8_string(int, char *);

static void handle_char(const char * & c, const Afm & afm, VxpsTextNode * & cur);

static bool needs_escape(const int ascii_code);

static bool nonprintable(int ascii_code);

static void handle_ligature(const LigatureInfo & lig, const Afm & afm, VxpsTextNode * & cur);

static void handle_kern_pair(const KPX & kp, const Afm & afm, VxpsTextNode * & cur);

static DocumentMedia default_media();


////////////////////////////////////////////////////////////////////////


inline int imax(int a, int b)  { return ( (a > b) ? a : b ); }
inline int imin(int a, int b)  { return ( (a < b) ? a : b ); }


////////////////////////////////////////////////////////////////////////

   //
   //  These should all be DIFFERENT.
   //
   //  List them alphabetically by value so
   //   it's easy to check.
   //

static const char * const   arcto_4pop_string = "ap";

static const char * const   setlinecap_string = "c";
static const char * const         clip_string = "cl";
static const char * const    closepath_string = "cp";

static const char * const       eofill_string = "eof";

static const char * const         fill_string = "f";

static const char * const      setgray_string = "g";
static const char * const     grestore_string = "gr";
static const char * const        gsave_string = "gs";

static const char * const     showpage_string = "h";
static const char * const  sethsbcolor_string = "hsv";

static const char * const  setlinejoin_string = "j";

static const char * const       lineto_string = "l";

static const char * const       moveto_string = "m";

static const char * const      newpath_string = "n";

static const char * const      rotate_string  = "r";
static const char * const      restore_string = "res";
static const char * const  setrgbcolor_string = "rgb";

static const char * const        scale_string = "s";

static const char * const       stroke_string = "t";
static const char * const    translate_string = "tr";

static const char * const setlinewidth_string = "w";

static const char * const setdash_string      = "d";


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PSfile
   //


////////////////////////////////////////////////////////////////////////


PSfile::PSfile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


PSfile::~PSfile()

{

close();

delete afm;  afm = (Afm *) 0;

}


////////////////////////////////////////////////////////////////////////


void PSfile::init_from_scratch()

{

File = (ofstream *) 0;

psout.ignore_columns = true;

Head = &psout;

afm = new Afm;

showpage_count = 0;

close();


return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::open(const char * filename, DocumentMedia DM, DocumentOrientation DO)

{

close();

File = new ofstream;

met_open(*File, filename);

if ( !(*File) )  {

   mlog << Error << "\nPSfile::open(const char *) -> "
        << "unable to open file \"" << filename << "\"\n\n";

   exit ( 1 );

}

OutputFilename = filename;

psout.attach(File);

psout.set_decimal_places(ml_prec);

Head = &psout;

   //
   //  set media first, then orientation
   //

set_media(DM);

set_orientation(DO);

File->setf(ios::fixed);

do_prolog();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::open(const char * filename)   //  uses default media and portrait orientation

{

open(filename, default_media(), default_orientation);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::open(const char * filename, DocumentOrientation DO)   //  uses default media

{

open(filename, default_media(), DO);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::do_prolog()

{

ConcatString orientation_string;
ConcatString media_string;
// ofstream & f = *File;
PSFilter & f = *Head;


switch ( Orientation )  {

   case OrientationPortrait :  orientation_string = "Portrait";   break;
   case OrientationLandscape:  orientation_string = "Landscape";  break;
   
   default:
      mlog << Error << "\nvoid PSfile::do_prolog() -> bad document orientation ... "
           << documentorientation_to_string(Orientation) << "\n\n";
      exit ( 1 );
      break;

}

switch ( Media )  {

   case MediaLetter:  media_string = "Letter";  break;
   case MediaA4:      media_string = "A4";      break;

   default:
      mlog << Error << "\nvoid PSfile::do_prolog() -> bad document media ... "
           << documentmedia_to_string(Media) << "\n\n";
      exit ( 1 );
      break;

}

media_string << ' ' << MediaWidth << ' ' << MediaHeight << " 0 () ()";

   //
   //  magic cookie
   //

f << "%!PS-Adobe-3.0\n";

   //
   //  comments
   //

f << "%%Title: "   << get_short_name(output_filename().c_str()) << '\n';

f << "%%Creator: MET Graphics\n";

f << "%%DocumentMedia: " << media_string << '\n';

f << "%%Pages: (atend)\n";

f << "%%PageOrder: Ascend\n";

f << "%%Orientation: " << orientation_string << '\n';

f << "%%EndComments\n";

   //
   //  prolog
   //

f << "%%BeginProlog\n"

  << '/' << setlinewidth_string << " {setlinewidth} def\n"
  << '/' << setdash_string      << " {setdash} def\n"
  << '/' << lineto_string       << " {lineto} def\n"
  << '/' << newpath_string      << " {newpath} def\n"
  << '/' << moveto_string       << " {moveto} def\n"
  << '/' << scale_string        << " {scale} def\n"
  << '/' << setlinecap_string   << " {setlinecap} def\n"
  << '/' << setlinejoin_string  << " {setlinejoin} def\n"
  << '/' << restore_string      << " {restore} def\n"
  << '/' << showpage_string     << " {showpage} def\n"
  << '/' << stroke_string       << " {stroke} def\n"
  << '/' << setgray_string      << " {setgray} def\n"
  << '/' << eofill_string       << " {eofill} def\n"
  << '/' << arcto_4pop_string   << " {arcto pop pop pop pop} def\n"
  << '/' << translate_string    << " {translate} def\n"
  << '/' << rotate_string       << " {rotate} def\n"
  << '/' << closepath_string    << " {closepath} def\n"
  << '/' << fill_string         << " {fill} def\n"
  << '/' << setrgbcolor_string  << " {setrgbcolor} def\n"
  << '/' << sethsbcolor_string  << " {sethsvcolor} def\n"
  << '/' << gsave_string        << " {gsave} def\n"
  << '/' << grestore_string     << " {grestore} def\n"
  << '/' << clip_string         << " {clip} def\n"

  << "%%EndProlog\n"
  << "\n\n";

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::close()

{

current_font = -1;
font_size    = -1.0;

Head->eod();

Head = &psout;

psout.detach();

if ( File )  {

   (*File) << "%%Trailer\n";

   if ( showpage_count > 0 )  (*File) << "%%Pages: " << showpage_count << '\n';

   (*File) << "%%EOF\n";

   File->close();

   delete File;   File = (ofstream *) 0;

}

afm->clear();

Orientation = default_orientation;

Family = default_font_family;

Media = default_media();

OutputFilename.clear();

showpage_count = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::set_font_size(double s)

{

choose_font(current_font, s);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::choose_font(int n, double s)

{

ConcatString data_dir;

data_dir = replace_path(default_met_data_dir);

choose_font_with_dir(n, s, data_dir.c_str());

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::choose_font_with_dir(int n, double s, const char * data_dir)

{

   //
   //  check values
   //

if ( (n < 0) || (n >= total_vx_ps_fonts) )  {

   mlog << Error << "\nPSfile::choose_font_with_dir() -> "
        << "bad font number -> " << n << "\n\n";

   exit ( 1 );

}

if ( s <= 0.0 )  {

   mlog << Error << "\nPSfile::choose_font_with_dir() -> "
        << "bad font size -> " << s << "\n\n";

   exit ( 1 );

}

ConcatString path;
char junk[256];

   //
   //  read afm file
   //

snprintf(junk, sizeof(junk), "%02d", n);

path << data_dir << '/' << afm_directory << '/' << junk << ".afm";

if ( !(afm->read(path)) )  {

   mlog << Error << "\nPSfile::choose_font_with_dir() -> "
        << "unable to read afm file \"" << path << "\"\n\n";

   exit ( 1 );

}


current_font = n;
font_size    = s;

   //
   //  write font selection/size commands into ps file
   //

// (*File) << "/" << (afm->FontName) << " findfont " << font_size << " scalefont setfont\n";
(*Head) << "/" << (afm->FontName) << " findfont " << font_size << " scalefont setfont\n";

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::write_centered_text(int center, int fill_flag,
              double x, double y, double x_offset, double y_offset, const char * s, const bool render_flag)

   //
   //  center      =  1   <==>   Text is centered horizontally
   //
   //              =  2   <==>   Text is centered both horizontally
   //                            and vertically
   //
   //
   //  fill_flag   =  0   <==>   Text is rendered in outline only
   //
   //              =  1   <==>   Text is solid-filled
   //

{

   //
   //  sanity check the values
   //

if ( (center != 1) && (center != 2) )  {

   mlog << Error << "\nPSfile::write_centered_text() -> "
        << "center flag must be 1 or 2\n\n";

   exit ( 1 );

}


if ( (fill_flag != 0) && (fill_flag != 1) )  {

   mlog << Error << "\nPSfile::write_centered_text() -> "
        << "fill flag must be 0 or 1\n\n";

   exit ( 1 );

}

if ( ! s )  s = na_str;   //  AAA


double L, R, B, T;
const double scale = 0.001*font_size;
double x_cur, y_cur;
double dx, dy;
VxpsTextNode node;
VxpsTextNode * n = (VxpsTextNode *) 0;


make_list(current_font, font_size, *afm, node, s);


B = node.total_bottom();

T = node.total_top();

L = node.total_left();

R = node.total_right();


x_cur = x;
y_cur = y;

   //
   //  delta x
   //

dx = -x_offset*(R - L);

dx -= L;   //  "moveto" command must move to first character origin, not L

   //
   //  delta y
   //

if ( center == 1 )   dy = 0.0;
else                 dy = -B - y_offset*(T - B);

x_cur += scale*dx;
y_cur += scale*dy;

   //
   //  initial moveto
   //

// File->precision(5);
//
// file() << x_cur << " " << y_cur << " m \n";


   //
   //  loop through the nodes
   //

if ( render_flag )  {

   n = &node;

   while ( n )  {

      // x_cur -= scale*(n->dx());
         x_cur += scale*(n->dx());

      write_single_node(n, x_cur, y_cur, fill_flag);

      x_cur += scale*(n->width());

      n = n->next;

   }

}


   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::write_single_node(const VxpsTextNode * n, double x_cur, double y_cur, int fill_flag)

{

if ( n->is_empty() )  return;

int j;
char junk[32];
const char * t = n->text();
// const double scale = 0.001*(n->font_size());
PSFilter & f = *Head;


file() << x_cur << ' ' << y_cur << ' ' << moveto_string << ' ';

file() << "(";


for (j=0; j<(n->nchars()); ++j)  {

   if ( needs_escape(t[j]) )  {

      f << '\\';

      f << t[j];

   } else if ( nonprintable(t[j]) )  {

      base_8_string(t[j], junk);

      f << '\\';

      f << junk;

   } else {

      f << t[j];

   }

}

file() << ") ";

switch ( fill_flag )  {

   case  0:  file() << "true charpath stroke";   break;
   case  1:  file() << "show";                   break;

   default:
      mlog << Error << "\nPSfile::write_single_node() -> "
           << "unrecognized fill flag: \"" << fill_flag << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch

f << '\n';

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::moveto(double x, double y)

{

// File->precision(5);
// File->precision(ml_prec);

file() << ' ' << x << ' ' << y << ' ' << moveto_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::translate(double dx, double dy)

{

// File->precision(5);

file() << ' ' << dx << ' ' << dy << ' ' << translate_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::rotate(double angle)

{

// File->precision(5);

file() << ' ' << angle << ' ' << rotate_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::newpath()

{

file() << ' ' << newpath_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::lineto(double x, double y)

{

// File->precision(5);
// File->precision(ml_prec);

file() << ' ' << x << ' ' << y << ' ' << lineto_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::stroke()

{

file() << ' ' << stroke_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::closepath()

{

file() << ' ' << closepath_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::fill()

{

file() << ' ' << fill_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::line(double x1, double y1, double x2, double y2, bool do_newpath)

{

if ( do_newpath )  newpath();

// File->precision(5);

file() << ' ' << x1 << ' ' << y1 << ' ' << moveto_string
       << ' ' << x2 << " " << y2 << ' ' << lineto_string
       << '\n';

if ( do_newpath )  stroke();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::setgray(double g)

{

const int d = Head->DecimalPlaces;

Head->set_decimal_places(3);

file() << ' ' << g << ' ' << setgray_string << '\n';

Head->set_decimal_places(d);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::setlinewidth(double w)

{

// File->precision(2);

file() << ' ' << w << ' ' << setlinewidth_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::setdash(const char *d)

{

// File->precision(2);

file() << ' ' << d << ' ' << setdash_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::showpage()

{

file() << "\n\n"
       << showpage_string << '\n'
       << "%%PageTrailer\n";

current_font = -1;
font_size    = -1.0;

afm->clear();

++showpage_count;

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::pagenumber(int page)

{

file() << "%%Page: " << page << ' ' << page << '\n';

if ( is_landscape() )  {

   // (*File) << " 90 " << rotate_string << " 0 " << (-MediaWidth) << ' ' << translate_string << '\n';
   (*Head) << " 90 " << rotate_string << " 0 " << (-MediaWidth) << ' ' << translate_string << '\n';

}


return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::box(double x_ll, double y_ll, double width, double height, double radius)

{

double ax, ay, bx, by, cx, cy, dx, dy;

ax = x_ll;
ay = y_ll + height;

bx = x_ll + width;
by = y_ll + height;

cx = x_ll + width;
cy = y_ll;

dx = x_ll;
dy = y_ll;

newpath();

moveto(ax + radius, ay);

file() << bx << ' ' << by << ' ' << cx << ' ' << cy << ' ' << radius << ' ' << arcto_4pop_string << '\n';
file() << cx << ' ' << cy << ' ' << dx << ' ' << dy << ' ' << radius << ' ' << arcto_4pop_string << '\n';
file() << dx << ' ' << dy << ' ' << ax << ' ' << ay << ' ' << radius << ' ' << arcto_4pop_string << '\n';
file() << ax << ' ' << ay << ' ' << bx << ' ' << by << ' ' << radius << ' ' << arcto_4pop_string << '\n';

stroke();

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::setrgbcolor(double r, double g, double b)

{

const int d = Head->DecimalPlaces;

Head->set_decimal_places(4);

// File->precision(5);

file() << ' ' << r << ' ' << g << ' ' << b << ' ' << setrgbcolor_string << '\n';

Head->set_decimal_places(d);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::sethsbcolor(double h, double s, double b)

{

// File->precision(5);

file() << ' ' << h << ' ' << s << ' ' << b << ' ' << sethsbcolor_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::gsave()

{

file() << ' ' << gsave_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::grestore()

{

file() << ' ' << grestore_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::comment(const char * text)

{

if ( empty(text) )  {

   mlog << Error << "\n\n  PSfile::comment(const char *) -> empty string!\n\n";

   exit ( 1 );

}

file() << "   %\n"
       << "   %  comment, page " << (showpage_count + 1) << ": " << text << "\n"
       << "   %\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::circle(double cx, double cy, double radius, bool stroke_flag)

{

// File->precision(5);

newpath();

file() << ' ' << cx << ' ' << cy << ' ' << radius << " 0 360 arc\n";

if ( stroke_flag )   stroke();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::clip()

{

file() << ' ' << clip_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::set_media(DocumentMedia m)

{

Media = m;

switch ( Media )  {

   case MediaLetter:
      MediaWidth  = Letter_pagewidth;
      MediaHeight = Letter_pageheight;
      break;

   case MediaA4:
      MediaWidth  = A4_pagewidth;
      MediaHeight = A4_pageheight;
      break;

   default:
      mlog << Error << "\nPSfile::set_media(DocumentMedia) -> bad media size ... "
           << documentmedia_to_string(Media) << "\n\n";
      exit ( 1 );
      break;

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::set_orientation(DocumentOrientation DO)

{

Orientation = DO;

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::begin_flate()

{

PSFilter **v = &fa_bank;
PSOutputFilter * pso = 0;

comment("begin flate compression");

   //
   //  add a flate encode filter
   //

*v = new FlateEncodeFilter();
 v = &((*v)->next);

   //
   //  add a ascii85 encode filter
   //

*v = new ASCII85EncodeFilter();
 v = &((*v)->next);

   //
   //  add a ps output filter
   //

pso = new PSOutputFilter();
pso->ignore_columns = false;
pso->file = File;

*v = pso;
 v = &((*v)->next);

   //
   //
   //

(*File) << "currentfile\n"
           "/ASCII85Decode filter /FlateDecode filter\n"
           "cvx exec\n";

Head = fa_bank;

Head->set_decimal_places(ml_prec);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::end_flate()

{


fa_bank->eod();

delete fa_bank;  fa_bank = 0;

Head = &psout;

psout.file = File;

file() << '\n';

comment("end flate compression");

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::outline_box(const Box & b)

{

newpath();

moveto(b.left() , b.bottom());
lineto(b.right(), b.bottom());
lineto(b.right(), b.top());
lineto(b.left(),  b.top());

closepath();

stroke();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::outline_box(const Box & b, double linewidth)

{

gsave();

   setlinewidth(linewidth);

   newpath();

   moveto(b.left() , b.bottom());
   lineto(b.right(), b.bottom());
   lineto(b.right(), b.top());
   lineto(b.left(),  b.top());

   closepath();

   stroke();

grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::outline_box(const Box & b, const Color & c, double linewidth)

{

gsave();

   set_color(c);

   setlinewidth(linewidth);

   outline_box(b);

grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::fill_box(const Box & b, const Color & c)

{

gsave();

   set_color(c);

   newpath();

   moveto(b.left() , b.bottom());
   lineto(b.right(), b.bottom());
   lineto(b.right(), b.top());
   lineto(b.left(),  b.top());

   closepath();

   fill();

grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::set_color(const Color & c)

{

double r, g, b;


if ( c.is_gray() )  {

   g = (c.green())/255.0;

   setgray(g);

} else {

   r = (c.red())/255.0;
   g = (c.green())/255.0;
   b = (c.blue())/255.0;

   setrgbcolor(r, g, b);

}

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::setlinejoin(int k)

{

   //
   //  valid values are 0, 1, 2
   //

if ( (k < 0) || (k > 2) )  {

   mlog << Error << "\n\n  PSfile::setlinejoin(int) -> invalid value for line join\n\n";

   exit ( 1 );

}

(*Head) << ' ' << k << ' ' << setlinejoin_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::setlinecap(int k)

{

   //
   //  valid values are 0, 1, 2
   //

if ( (k < 0) || (k > 2) )  {

   mlog << Error << "\n\n  PSfile::setlinecap(int) -> invalid value for line cap\n\n";

   exit ( 1 );

}

(*Head) << ' ' << k << ' ' << setlinecap_string << '\n';

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::set_family(FontFamily f)

{


switch ( f )  {

   case ff_Helvetica:    //  fall through
   case ff_NewCentury:   //  fall through
   case ff_Palatino:     //  fall through
   case ff_Times:        //  fall through
   case ff_Courier:      //  fall through
   case ff_Bookman:      //  fall through
      Family = f;
      break;

   default:
      mlog << Error << "\n\n  PSfile::set_family(FontFamily) -> bad font family ... "
           << fontfamily_to_string(f) << "\n\n";
      exit ( 1 );
      break;

}   //  switch

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::roman()

{

roman(font_size);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::bold()

{

bold(font_size);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::italic()

{

italic(font_size);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::bolditalic()

{

bolditalic(font_size);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::roman(double size)

{

int n;

n = ff_to_roman(Family);

choose_font(n, size);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::italic(double size)

{

int n;

n = ff_to_italic(Family);

choose_font(n, size);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::bold(double size)

{

int n;

n = ff_to_bold(Family);

choose_font(n, size);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::bolditalic(double size)

{

int n;

n = ff_to_bolditalic(Family);

choose_font(n, size);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::dingbats(double size)

{

choose_font(33, size);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::symbol(double size)

{

choose_font(27, size);

return;

}


////////////////////////////////////////////////////////////////////////


int PSfile::roman_font() const

{

return ( ff_to_roman (Family) );

}


////////////////////////////////////////////////////////////////////////


int PSfile::italic_font() const

{

return ( ff_to_italic(Family) );

}


////////////////////////////////////////////////////////////////////////


int PSfile::bold_font() const

{

return ( ff_to_bold(Family) );

}


////////////////////////////////////////////////////////////////////////


int PSfile::bolditalic_font() const

{

return ( ff_to_bolditalic (Family) );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void base_8_string(int k, char * out)

{

if ( k < 0 )  k += 256;

char junk[256];
int pos = sizeof(junk) - 1;

junk[pos] = (char) 0;

while ( k )  {

   --pos;

   junk[pos] = '0' + k%8;

   k /= 8;

}


strcpy(out, junk + pos);

return;

}


////////////////////////////////////////////////////////////////////////


void make_list(const int font_number, const double font_size, const Afm & afm,
               VxpsTextNode & node, const char * s)

{

VxpsTextNode * cur = &node;
const char * c = s;


cur->set_font_number(font_number);

cur->set_font_size(font_size);


do {

   handle_char(c, afm, cur);

} while ( *c );


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void handle_char(const char * & c, const Afm & afm, VxpsTextNode * & cur)

{

int ascii_code = *c;

if ( ascii_code < 0 )  ascii_code += 256;

   //
   //   get char metric info
   //

int j;

j = afm.lookup_cm(ascii_code);

if ( j < 0 )  {

   mlog << Error << "\nhandle_char() -> "
        << "no char metric for ascii code " << ascii_code
        << " in font " << (afm.FontName) << "\n\n";

   exit ( 1 );

}

AfmCharMetrics & cm = afm.cm[j];

   //
   //  ligature?
   //

LigatureInfo lig;

if ( c[1] && afm.has_ligature(c[0], c[1], lig) )  {

   handle_ligature(lig, afm, cur);

   c += 2;

   return;

}

   //
   //  is it the start of a kern pair?
   //

KPX kp;

if ( c[1] && afm.has_kern_pair(c[0], c[1], kp) )  {

   cur->add_char(cm);

   handle_kern_pair(kp, afm, cur);

   ++c;

   return;

}

   //
   //  nope, just regular character
   //

cur->add_char(cm);

++c;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool needs_escape(const int ascii_code)

{

bool ans = false;

switch ( ascii_code )  {

   case '\\':
   case '(':
   case ')':
      ans = true;
      break;

   default:
      ans = false;
      break;

};

return ( ans );

}


////////////////////////////////////////////////////////////////////////


bool nonprintable(int ascii_code)

{

if ( ascii_code < 0 )  ascii_code += 256;

if ( ascii_code < 32 )  return ( true );

if ( ascii_code > 126 )  return ( true );

return ( false );

}


////////////////////////////////////////////////////////////////////////


void handle_ligature(const LigatureInfo & lig, const Afm & afm, VxpsTextNode * & cur)

{

cur->add_char(afm.cm[lig.ligature_index]);


return;

}


////////////////////////////////////////////////////////////////////////


void handle_kern_pair(const KPX & kp, const Afm & afm, VxpsTextNode * & cur)

{

if ( !(cur->is_empty()) )  {

   cur->add_link();

   cur = cur->next;

}

cur->set_dx(kp.dx);


return;

}


////////////////////////////////////////////////////////////////////////


DocumentMedia default_media()

{

ConcatString cs;

if ( !get_env(papersize_env, cs) )  return ( MediaLetter );

if ( cs == "Letter" )  return ( MediaLetter );

if ( cs == "A4" )  return ( MediaA4 );

mlog << Error << "\ndefault_media() -> "
     << "bad value \"" << cs << "\" for environment variable "
     << papersize_env << "\n\n";

exit ( 1 );

return ( no_document_media );

}


////////////////////////////////////////////////////////////////////////


int ff_to_roman(const FontFamily f)

{

int n = 0;


switch ( f )  {

   case ff_Helvetica:    n = 11;  break;
   case ff_NewCentury:   n = 22;  break;
   case ff_Palatino:     n = 26;  break;

   case ff_Times:        n = 31;  break;
   case ff_Courier:      n =  7;  break;
   case ff_Bookman:      n =  5;  break;

   default:
      mlog << Error << "\n\n  ff_to_roman() -> bad font family ... "
           << fontfamily_to_string(f) << "\n\n";
      exit ( 1 );
      break;

}   //  switch


return ( n );

}


////////////////////////////////////////////////////////////////////////


int ff_to_italic(const FontFamily f)

{

int n = 0;


switch ( f )  {

   case ff_Helvetica:    n = 18;  break;
   case ff_NewCentury:   n = 21;  break;
   case ff_Palatino:     n = 25;  break;

   case ff_Times:        n = 30;  break;
   case ff_Courier:      n = 10;  break;
   case ff_Bookman:      n =  6;  break;

   default:
      mlog << Error << "\n\n  ff_to_italic() -> bad font family ... "
           << fontfamily_to_string(f) << "\n\n";
      exit ( 1 );
      break;

}   //  switch


return ( n );

}


////////////////////////////////////////////////////////////////////////


int ff_to_bold(const FontFamily f)

{

int n = 0;


switch ( f )  {

   case ff_Helvetica:    n = 12;  break;
   case ff_NewCentury:   n = 19;  break;
   case ff_Palatino:     n = 23;  break;

   case ff_Times:        n = 28;  break;
   case ff_Courier:      n =  8;  break;
   case ff_Bookman:      n =  3;  break;

   default:
      mlog << Error << "\n\n  ff_to_bold() -> bad font family ... "
           << fontfamily_to_string(f) << "\n\n";
      exit ( 1 );
      break;

}   //  switch


return ( n );

}


////////////////////////////////////////////////////////////////////////


int ff_to_bolditalic(const FontFamily f)

{

int n = 0;


switch ( f )  {

   case ff_Helvetica:    n = 13;  break;
   case ff_NewCentury:   n = 20;  break;
   case ff_Palatino:     n = 24;  break;

   case ff_Times:        n = 29;  break;
   case ff_Courier:      n =  9;  break;
   case ff_Bookman:      n =  4;  break;

   default:
      mlog << Error << "\n\n  ff_to_bolditalic() -> bad font family ... "
           << fontfamily_to_string(f) << "\n\n";
      exit ( 1 );
      break;

}   //  switch


return ( n );

}


////////////////////////////////////////////////////////////////////////





