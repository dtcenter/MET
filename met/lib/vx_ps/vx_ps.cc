// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


static const int verbose = 0;


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

#include "vx_ps/vx_ps.h"


////////////////////////////////////////////////////////////////////////


static const char Gs          [] = "/usr/bin/gs";

static const char Gs_options  [] = "-dQUIET -dNOPAUSE -dBATCH";

static const char Gs_device   [] = "bbox";


////////////////////////////////////////////////////////////////////////


static const char temp_filename[]    = "yabba_eps_temp";


////////////////////////////////////////////////////////////////////////


static void ps_bbox(const char * filename, Ps_Bbox & lo_res, Ps_Bbox & hi_res);

static void base_8_string(int, char *);

static void handle_char(const char * & c, const Afm & afm, VxpsTextNode * & cur);

static int  needs_escape(const int ascii_code);

static int  nonprintable(int ascii_code);

static void handle_ligature(const LigatureInfo & lig, const Afm & afm, VxpsTextNode * & cur);

static void handle_kern_pair(const KPX & kp, const Afm & afm, VxpsTextNode * & cur);


////////////////////////////////////////////////////////////////////////


inline int imax(int a, int b)  { return ( (a > b) ? a : b ); }
inline int imin(int a, int b)  { return ( (a < b) ? a : b ); }


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

afm = new Afm;

is_eps = 0;   //  needed for close();

close();


return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::open(const char * filename)

{

close();

File = new ofstream;

File->open(filename);

if ( !(*File) )  {

   cerr << "\n\n  PSfile::open(const char *) -> unable to open file \""
        << filename << "\"\n\n";

   exit ( 1 );

}

File->setf(ios::fixed);

(*File) << "%!PS-Adobe-3.0\n"
        << "%%EndComments\n"

        << "\n\n"

        << "%%BeginProlog\n"

        << "/w {setlinewidth} def\n"
        << "/l {lineto} def\n"
        << "/n {newpath} def\n"
        << "/m {moveto} def\n"
        << "/s {scale} def\n"
        << "/c {setlinecap} def\n"
        << "/j {setlinejoin} def\n"
        << "/r {restore} def\n"
        << "/h {showpage} def\n"
        << "/t {stroke} def\n"
        << "/g {setgray} def\n"
        << "/f {eofill} def\n"
        << "/ap { arcto pop pop pop pop } bind def\n"

        << "%%EndProlog\n"

        << "\n\n";




return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::eps_open(const char * filename)

{

open(temp_filename);

is_eps = 1;

eps_output_filename = new char [1 + strlen(filename)];

if ( !eps_output_filename )  {

   cerr << "\n\n  PSfile::eps_open() -> memory allocation error\n\n";

   exit ( 1 );

}

strcpy(eps_output_filename, filename);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::close()

{

current_font = -1;
font_size    = -1.0;

if ( File )  {

   (*File) << "\n%%EOF\n";

   File->close();

   delete File;   File = (ofstream *) 0;

}

if ( is_eps )  finish_eps();

is_eps = 0;

afm->clear();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::finish_eps()

{

if ( !is_eps )  return;

int j;
int saw_newline;
char command[512];
char Line[128];
FILE * p = (FILE *) 0;
ifstream i;
ofstream f;
char c;
const int line_offset = 14;

   //
   //  get bbox
   //

sprintf(command, "%s -sDEVICE=%s %s %s 2>&1 | grep -v HiRes", Gs, Gs_device, Gs_options, temp_filename);

if ( verbose )   cout << "finish_eps() -> command = " << command << "\n" << flush;

if ( (p = popen(command, "r")) == NULL )  {

   cerr << "\n\n  PSfile::finish_eps() -> can't create pipe\n\n";

   exit ( 1 );

}

if ( fgets(Line, sizeof(Line), p) == NULL )  {

   cerr << "\n\n  PSfile::finish_eps() -> can't read pipe\n\n";

   exit ( 1 );

}

pclose(p);   p = (FILE *) 0;

// j = sscanf(Line, "%%BoundingBox: %d %d %d %d", &bbox_x_ll, &bbox_y_ll, &bbox_x_ur, &bbox_y_ur);

j = sscanf(Line + line_offset, "%d%d%d%d", &bbox_x_ll, &bbox_y_ll, &bbox_x_ur, &bbox_y_ur);

if ( j != 4 )  {

   cerr << "\n\n  PSfile::finish_eps() -> can't read pipe output (j = " << j << ")... \"" << (Line + line_offset) << "\"\n\n";

   exit ( 1 );

}

   //
   //  open output file
   //

f.open(eps_output_filename);

if ( !f )  {

   cerr << "\n\n  PSfile::finish_eps() -> unable to open output file \"" << eps_output_filename << "\"\n\n";

   exit ( 1 );

}

   //
   //  write first line
   //

f << "%!PS-Adobe-3.0 EPSF-3.0\n";

   //
   //  write bounding box
   //

f << "%%BoundingBox: " << bbox_x_ll << " " << bbox_y_ll << " " << bbox_x_ur << " " << bbox_y_ur << "\n";

   //
   //  copy all but first line of temp file to output
   //

i.open(temp_filename);

if ( !i )  {

   cerr << "\n\n  PSfile::finish_eps() -> can't open temp file for reading\n\n";

   exit ( 1 );

}

saw_newline = 0;

while ( i.get(c) )  {

   if ( saw_newline )  f.put(c);

   if ( (saw_newline == 0) && (c == '\n') )  saw_newline = 1;

}

i.close();

   //
   //  close output file
   //

f.close();

   //
   //  delete eps_output_filename
   //

delete [] eps_output_filename;  eps_output_filename = (char *) 0;

   //
   //  remove temp file
   //

if ( unlink(temp_filename) < 0 )  {

   cerr << "\n\n  PSfile::finish_eps() -> can't remove temp file\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::choose_font(int n, double s)

{

char data_dir[PATH_MAX];

sprintf(data_dir, "%s/data", MET_BASE);

choose_font(n, s, data_dir);

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::choose_font(int n, double s, const char *data_dir)

{

   //
   //  check values
   //

if ( (n < 0) || (n >= total_vx_ps_fonts) )  {

   cerr << "\n\n  PSfile::choose_font() -> bad font number -> " << n << "\n\n";

   exit ( 1 );

}

if ( s <= 0.0 )  {

   cerr << "\n\n  PSfile::choose_font() -> bad font size -> " << s << "\n\n";

   exit ( 1 );

}

char path[PATH_MAX];

   //
   //  read afm file
   //

sprintf(path, "%s/%s/%02d.afm", data_dir, afm_directory, n);

if ( !(afm->read(path)) )  {

   cerr << "\n\n  PSfile::choose_font() -> unable to read afm file \"" << path << "\"\n\n";

   exit ( 1 );

}


current_font = n;
font_size    = s;

   //
   //  write font selection/size commands into ps file
   //

(*File) << "/" << (afm->FontName) << " findfont " << font_size << " scalefont setfont\n";

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::write_centered_text(int center, int fill_flag,
              double x, double y, double x_offset, double y_offset, const char * s)

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

   cerr << "\n\n  PSfile::write_centered_text() -> center flag must be 1 or 2\n\n";

   exit ( 1 );

}


if ( (fill_flag != 0) && (fill_flag != 1) )  {

   cerr << "\n\n  PSfile::write_centered_text() -> fill flag must be 0 or 1\n\n";

   exit ( 1 );

}



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

n = &node;

while ( n )  {

   // x_cur -= scale*(n->dx());
      x_cur += scale*(n->dx());

   write_single_node(n, x_cur, y_cur, fill_flag);

   x_cur += scale*(n->width());

   n = n->next;

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


file() << x_cur << " " << y_cur << " moveto ";

file() << "(";


for (j=0; j<(n->nchars()); ++j)  {

   if ( needs_escape(t[j]) )  {

      File->put('\\');

      File->put(t[j]);

   } else if ( nonprintable(t[j]) )  {

      base_8_string(t[j], junk);

      File->put('\\');

      file() << junk;

   } else {

      File->put(t[j]);

   }

}

file() << ") ";

switch ( fill_flag )  {

   case  0:  file() << "true charpath stroke";   break;
   case  1:  file() << "show";                   break;

   default:
      cerr << "\n\n  PSfile::write_single_node() -> unrecognized fill flag: \""
           << fill_flag << "\"\n\n";
      exit ( 1 );
      break;

}   //  switch

File->put('\n');

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::moveto(double x, double y)

{

File->precision(5);

file() << " " << x << " " << y << " m\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::translate(double dx, double dy)

{

File->precision(5);

file() << " " << dx << " " << dy << " translate\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::rotate(double angle)

{

File->precision(5);

file() << " " << angle << " rotate\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::newpath()

{

file() << " n\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::lineto(double x, double y)

{

File->precision(5);

file() << " " << x << " " << y << " l\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::stroke()

{

file() << " t\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::closepath()

{

file() << " closepath\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::fill()

{

file() << " fill\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::line(double x1, double y1, double x2, double y2, int new_path_flag)

{

if ( new_path_flag )  newpath();

File->precision(5);

file() << " " << x1 << " " << y1 << " m " << x2 << " " << y2 << " l\n";

if ( new_path_flag )  stroke();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::setgray(double g)

{

File->precision(2);

file() << " " << g << " setgray\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::setlinewidth(double w)

{

File->precision(2);

file() << " " << w << " w\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::showpage()

{

file() << "\n\nshowpage\n%%PageTrailer\n\n";

current_font = -1;
font_size    = -1.0;

afm->clear();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::pagenumber(int page)

{

file() << "%%Page: x " << page << "\n";

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

file() << bx << " " << by << " " << cx << " " << cy << " " << radius << " ap\n";
file() << cx << " " << cy << " " << dx << " " << dy << " " << radius << " ap\n";
file() << dx << " " << dy << " " << ax << " " << ay << " " << radius << " ap\n";
file() << ax << " " << ay << " " << bx << " " << by << " " << radius << " ap\n";

stroke();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::setrgbcolor(double r, double g, double b)

{

File->precision(5);

file() << " " << r << " " << g << " " << b << " setrgbcolor\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::sethsbcolor(double h, double s, double b)

{

File->precision(5);

file() << " " << h << " " << s << " " << b << " sethsbcolor\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::gsave()

{

file() << " gsave\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::grestore()

{

file() << " grestore\n";

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::circle(double cx, double cy, double radius, int stroke_flag)

{

File->precision(5);

newpath();

file() << " " << cx << " " << cy << " " << radius << " 0 360 arc\n";

if ( stroke_flag )   stroke();

return;

}


////////////////////////////////////////////////////////////////////////


void PSfile::clip()

{

file() << " clip\n";

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


Ps_Bbox lo_res_bbox(const char *filename)

{

Ps_Bbox Hi, Lo;

ps_bbox(filename, Lo, Hi);

return ( Lo );

}


////////////////////////////////////////////////////////////////////////


Ps_Bbox hi_res_bbox(const char *filename)

{

Ps_Bbox Hi, Lo;

ps_bbox(filename, Lo, Hi);

return ( Hi );

}


////////////////////////////////////////////////////////////////////////


void ps_bbox(const char * filename, Ps_Bbox & lo_res, Ps_Bbox & hi_res)

{

int j;
FILE * pipe = (FILE *) 0;
char command[256];
char line[256];


   //
   //  construct the command
   //

// sprintf(command, "%s -sDEVICE=%s %s %s | & cat", gs, gs_device, gs_options, filename);   //  this works for csh

   sprintf(command, "%s -sDEVICE=%s %s %s 2>&1", Gs, Gs_device, Gs_options, filename);   //  this works for sh (bash)

   //
   //  open the pipe
   //

pipe = popen(command, "r");

if ( pipe == NULL )  {

   cerr << "\n\n  bbox(const char *filename) -> failed to open pipe\n\n";

   exit ( 1 );

}

   //
   //  get first line (lo-res bounding box)
   //

if ( fgets(line, sizeof(line), pipe) == NULL )  {

   cerr << "\n\n  bbox(const char *filename) -> read error on first line from pipe\n\n";

   exit ( 1 );

}

j = sscanf(line + 15, "%lf%lf%lf%lf", &(lo_res.left),
                                      &(lo_res.bottom),
                                      &(lo_res.right),
                                      &(lo_res.top));

if ( j != 4 )  {

   cerr << "\n\n  bbox(const char *filename) -> sscanf error on first line from pipe\n\n";

   exit ( 1 );

}


   //
   //  get second line (hi-res bounding box)
   //

if ( fgets(line, sizeof(line), pipe) == NULL )  {

   cerr << "\n\n  bbox(const char *filename) -> read error on second line from pipe\n\n";

   exit ( 1 );

}

j = sscanf(line + 20, "%lf%lf%lf%lf", &(hi_res.left),
                                      &(hi_res.bottom),
                                      &(hi_res.right),
                                      &(hi_res.top));

if ( j != 4 )  {

   cerr << "\n\n  bbox(const char *filename) -> sscanf error on second line from pipe\n\n";

   exit ( 1 );

}

   //
   //  done
   //

pclose(pipe);   pipe = (FILE *) 0;

return;

}


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

   cerr << "\n\n  handle_char() -> no char metric for ascii code " << ascii_code
        << " in font " << (afm.FontName)
        << "\n\n";

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


int needs_escape(const int ascii_code)

{

int ans;

switch ( ascii_code )  {

   case '\\':
   case '(':
   case ')':
      ans = 1;
      break;


   default:
      ans = 0;
      break;

};



return ( ans );

}


////////////////////////////////////////////////////////////////////////


int nonprintable(int ascii_code)

{

if ( ascii_code < 0 )  ascii_code += 256;

if ( ascii_code < 32 )  return ( 1 );

if ( ascii_code > 126 )  return ( 1 );

return ( 0 );

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





