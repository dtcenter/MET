// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


static const int  dpi         = 72;

static const int fi_ligature  = 174;
static const int fl_ligature  = 175;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"

#include "cgraph_main.h"
#include "cgraph_font.h"
#include "cgraphbase_plottype_to_string.h"

#include "gs_ps_map.h"


////////////////////////////////////////////////////////////////////////


struct ft_user_info {

   CgraphBase * cgraph;

   bool have_path;

   double x_offset;
   double y_offset;

};


////////////////////////////////////////////////////////////////////////


struct Wct_Info {

   FT_UInt glyph_index;

   double kern;

   double x_bitmap_origin;
   double y_bitmap_origin;

   double x_outline_origin;
   double y_outline_origin;

};


static const int max_wct_infos = 256;

static Wct_Info wct_info [max_wct_infos];


////////////////////////////////////////////////////////////////////////


inline double fix_to_float(FT_Pos x) { return ( x/64.0 ); }

inline double my_min(double x, double y) { return ( (x < y) ? x : y ); }
inline double my_max(double x, double y) { return ( (x > y) ? x : y ); }


////////////////////////////////////////////////////////////////////////


static const FontFamily default_font_family = ff_NewCentury;


////////////////////////////////////////////////////////////////////////


static void remap_string(FT_Face, const char * in, const char * & out);

static int my_cubic  (const FT_Vector * control1, const FT_Vector * control2, const FT_Vector * to, void *);
static int my_conic  (const FT_Vector * control, const FT_Vector * to, void *);
static int my_lineto (const FT_Vector * to, void *);
static int my_moveto (const FT_Vector * to, void *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class CgraphBase
   //


////////////////////////////////////////////////////////////////////////


CgraphBase::CgraphBase()

{

cgraph_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


CgraphBase::~CgraphBase()

{

close();

}


////////////////////////////////////////////////////////////////////////


CgraphBase::CgraphBase(const CgraphBase &)

{

mlog << Error << "\n\n  CgraphBase::CgraphBase(const CgraphBase &) -> should never be called!\n\n";

exit ( 1 );

   // cgraph_init_from_scratch();
   // 
   // assign(c);

}


////////////////////////////////////////////////////////////////////////


CgraphBase & CgraphBase::operator=(const CgraphBase &)

{

mlog << Error << "\n\n  operator=CgraphBase(const CgraphBase &) -> should never be called!\n\n";

exit ( 1 );


// if ( this == &c )  return ( * this );
// 
// assign(c);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::cgraph_init_from_scratch()

{

Surface = (cairo_surface_t *) 0;

Cr = (cairo_t *) 0;

Ptype = no_cgraph_plot_type;

Filename.clear();

Library = (FT_Library) 0;

CurrentFont = 0;

DoKerning   = true;
DoLigatures = true;

Family = default_font_family;

LastTextWidth = 0.0;




   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::close()

{

if ( Cr )  { cairo_destroy (Cr);   Cr = (cairo_t *) 0; }

if ( Ptype != no_cgraph_plot_type )  write();

if ( Surface )  { cairo_surface_destroy (Surface);   Surface = (cairo_surface_t *) 0; }

Ptype = no_cgraph_plot_type;

CurrentFont = 0;

DoKerning   = true;
DoLigatures = true;

Fonts.clear();

if ( Library )  { FT_Done_FreeType(Library);  Library = (FT_Library) 0; }

Family = default_font_family;

LastTextWidth = 0.0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::write()

{

if ( Filename.empty() )  {

   mlog << Error << "\n\n  CgraphBase::write() -> no filename given!\n\n";

   exit ( 1 );

}

cairo_status_t cstatus = CAIRO_STATUS_SUCCESS;

switch ( Ptype )  {


   case png_type:
      cstatus = cairo_surface_write_to_png (Surface, Filename.c_str());
      if ( cstatus != CAIRO_STATUS_SUCCESS )  {
         mlog << Error << "\n\n  CgraphBase::write() -> trouble writing png file \""
              << Filename << "\" ... " << cairo_status_to_string(cstatus)
              << "\n\n";
         exit ( 1 );
      }
      break;


   case svg_type:
      // cairo_surface_write_to_svg (Surface, Filename);
      break;


         /////////////////////////////////////////////////

   case no_cgraph_plot_type:
      break;

   default:
      mlog << Error << "\n\n  CgraphBase::close() -> plot type "
           << cgraphbase_plottype_to_string(Ptype) 
           << " is not yet supported.\n\n";
      exit ( 1 );
      break;

}   //  switch

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::open(const char * filename, int width, int height, CgraphBase::PlotType t)

{

close();

if ( empty (filename) )  {

   mlog << Error << "\n\n  CgraphBase::open() -> empty filename!\n\n";

   exit ( 1 );

   // return ( false );

}

Filename = filename;

Filename.chomp(".ps");
Filename.chomp(".eps");
Filename.chomp(".png");
Filename.chomp(".svg");
Filename.chomp(".pdf");

Ptype = t;

MediaWidth  = width;
MediaHeight = height;

PageBox.set_llwh(0.0, 0.0, MediaWidth, MediaHeight);

switch ( Ptype )  {

   case png_type:
      Filename << ".png";
      Surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
      break;

   case svg_type:
      Filename << ".svg";
      Surface = cairo_svg_surface_create   (Filename.c_str(), width, height);
      break;

   default:
      mlog << Error << "\n\n  CgraphBase::open() -> plot type "
           << cgraphbase_plottype_to_string(Ptype) 
           << " is not yet supported\n\n";
      break;

}   //  switch


Cr = cairo_create (Surface);

   //
   //  some defaults I prefer
   //

setlinecap_round();
setlinejoin_round();

   //
   //  set up coordinate system
   //

// cairo_translate(Cr, 0.0, height);
// cairo_scale    (Cr, 1.0, -1.0);

   //
   //  initialize freetype
   //

int error = FT_Init_FreeType(&Library);

if ( error )   {

   mlog << Error << "\n\n  CgraphBase::cgraph_init_from_scratch() -> error initializing freetype library\n\n";

   exit ( 1 );

}


   //
   //  done
   //

// return ( true );
return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::set_do_kerning(bool tf)

{

DoKerning = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::set_do_ligatures(bool tf)

{

DoLigatures = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::set_color(const Color & c)

{

double R, G, B;

R = (c.red())/255.0;
G = (c.green())/255.0;
B = (c.blue())/255.0;

cairo_set_source_rgb (Cr, R, G, B);

if ( CS.depth() > 0 )  (void) CS.pop();

CS.push(c);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::setrgbcolor(double R, double G, double B)

{

cairo_set_source_rgb (Cr, R, G, B);

Color c;
int ir, ig, ib;

ir = nint(255.0*R);
ig = nint(255.0*G);
ib = nint(255.0*B);

if ( ir < 0 )  ir = 0;
if ( ig < 0 )  ig = 0;
if ( ib < 0 )  ib = 0;

if ( ir > 255 )  ir = 255;
if ( ig > 255 )  ig = 255;
if ( ib > 255 )  ib = 255;

c.set_rgb((unsigned char) ir, (unsigned char) ig, (unsigned char) ib);

if ( CS.depth() > 0 )  (void) CS.pop();

CS.push(c);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::setgray(double g)

{

cairo_set_source_rgb (Cr, g, g, g);

int k = nint(255.0*g);

Color c((unsigned char) k);

if ( CS.depth() > 0 )  (void) CS.pop();

CS.push(c);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::choose_font(int ps_font_number, double size)

{

if ( (ps_font_number < 0) || (ps_font_number >= total_vx_ps_fonts) )  {

   mlog << Error << "\n\n  CgraphBase::choose_font(int font_number, double size) -> bad font number ... "
        << ps_font_number << "\n\n";

   exit ( 1 );

}


if ( size <= 0.0 )  {

   mlog << Error << "\n\n  CgraphBase::choose_font(int font_number, double size) -> bad font size ... "
        << size << "\n\n";

   exit ( 1 );

}

   //
   //  check to see if it's already loaded
   //

CurrentFont = Fonts.lookup_by_ps_font_number(ps_font_number);

if ( ! CurrentFont  )  {

   load_ps_font(ps_font_number);

   CurrentFont = Fonts.lookup_by_ps_font_number(ps_font_number);   // this should work now

}

   //
   //  set size
   //

set_current_font_size(size);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::load_ps_font(int ps_font_number)

{

CgFont f;

f.full_pfb_name << f.gs_font_dir << '/'
                << gs_ps_map_info[ps_font_number].gs_pfb_name
                << ".pfb";

f.short_pfb_name = get_short_name(f.full_pfb_name.c_str());

f.full_afm_name << f.gs_font_dir << '/'
                << gs_ps_map_info[ps_font_number].gs_pfb_name
                << ".afm";

f.short_afm_name = get_short_name(f.full_afm_name.c_str());

f.ps_name = gs_ps_map_info[ps_font_number].ps_font_name;

f.orig_ps_size   = 10.0;   //  ???
f.scaled_ps_size = 10.0;   //  ???

f.ps_font_number = ps_font_number;

f.afm = new Afm;

if ( ! (f.afm->read(f.full_afm_name)) )  {

   mlog << Error << "\n\n  CgraphBase::load_ps_font(int) -> unable to load afm file \""
        << f.full_afm_name << "\"\n\n";

   exit ( 1 );

}

f.face = ft_load(f.full_pfb_name.c_str(), f.full_afm_name.c_str());



   //
   //  done
   //

Fonts.add_no_repeat(f);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::set_current_font_size(double size)

{

int error, char_size;

if ( !CurrentFont )  {

   mlog << Error << "\n\n  CgraphBase::set_current_font_size(double) -> no current font!\n\n";

   exit ( 1 );

}

char_size = nint(size*64.0);

error = FT_Set_Char_Size(CurrentFont->face, char_size, char_size, dpi, dpi);

if ( error )  {

   mlog << Error << "\n\n  CgraphBase::set_current_font_size(double) -> error setting font size\n\n";

   mlog << Error << "\n\n  Error = " << error << "\n\n";

   exit ( 1 );

}

CurrentFont->scaled_ps_size = size;


return;

}


////////////////////////////////////////////////////////////////////////


double CgraphBase::current_font_size() const

{

if ( !CurrentFont )  {

   mlog << Error 
        << "\n\n  CgraphBase::current_font_size() const -> no current font!\n\n";

   exit ( 1 );

}


return ( CurrentFont->scaled_ps_size );

}


////////////////////////////////////////////////////////////////////////


FT_Face CgraphBase::ft_load(const char * pfb_path, const char * afm_path)

{

int error;
FT_Face face = 0;

   //
   //  load font
   //

error = FT_New_Face(Library, pfb_path, 0, &face);

if ( error == FT_Err_Unknown_File_Format )  {

   mlog << Error << "\n\n  CgraphBase::ft_load() -> unknown file format\n\n";

   exit ( 1 );

} else if ( error )  {

   mlog << Error << "\n\n  CgraphBase::ft_load() -> error reading font file \""
        << pfb_path << "\" ... " << error << "\n\n";

   exit ( 1 );

}

   //
   //  attach the afm file
   //

error = FT_Attach_File(face, afm_path);

if ( error )  {

   mlog << Error << "\n\n  CgraphBase::ft_load() -> can't load afm file \"" << afm_path << "\"\n\n";

   exit ( 1 );

}

   //
   //  select the adobe charmap
   //

error = FT_Select_Charmap(face, FT_ENCODING_ADOBE_STANDARD);

if ( error )  {

   error = FT_Select_Charmap(face, FT_ENCODING_ADOBE_CUSTOM);

   if ( error )  {

      mlog << Error << "\n\n  CgraphBase::ft_load() -> can't select adobe encoding\n\n";

      exit ( 1 );

   }

}


   //
   //  done
   //

return ( face );

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::write_centered_text(int center, int fill_flag, double x_pin, double y_pin, double u, double v, const char c, const bool render_flag)

{

char junk[2];

junk[0] = c;
junk[1] = (char) 0;

write_centered_text(center, fill_flag, x_pin, y_pin, u, v, junk, render_flag);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::write_centered_text(int center, int fill_flag, double x_pin, double y_pin, double u, double v, const char * s, const bool render_flag)

{

   //
   //  sanity check some stuff
   //

if ( empty(s) )  {

   mlog << Error << "\n\n  CgraphBase::write_centered_text() -> empty string! (2)\n\n";

   exit ( 1 );

}

if ( CurrentFont == 0 )  {

   mlog << Error << "\n\n  CgraphBase::write_centered_text() -> no current font!\n\n";

   exit ( 1 );

}


   //
   //  do we really need all this junk?
   //

int j, k, error;
double L, R, B, T;
double x_page, y_page;
double x_char_origin;
FT_Face face = CurrentFont->face;   //  this should be nonzero
FT_UInt previous;
FT_Glyph_Metrics * metrics = (FT_Glyph_Metrics *) 0;
FT_Vector k_delta;
const bool use_kerning = DoKerning && FT_HAS_KERNING(face);
const char * new_string = (const char *) 0;
bool first_char = false;
bool last_char  = false;
double x_bearing, y_bearing, advance;
double char_width, char_height;
double delta_x, delta_y;
Wct_Info * info = wct_info;




if ( DoLigatures )  remap_string(face, s, new_string);
else                new_string = s;

const int N = strlen(new_string);

if ( N >= max_wct_infos )  {

   mlog << Error << "\n\n  CgraphBase::write_centered_text() -> "
        << "increase parameter \"max_wct_infos\" to at least "
        << N << "\n\n";

   exit ( 1 );

}

   //
   //  get the string bounding box, and 
   //     (relative) coordinates for all 
   //     the individual characters
   //

previous = 0;

L = 0.0;
R = 0.0;

B =  1.0e10;
T = -1.0e10;

x_char_origin = 0.0;

first_char = true;

info = wct_info;

for (j=0; j<N; ++j)  {

   last_char = (j == (N - 1));

   k = new_string[j];

   if ( k < 0 )  k += 256;  //  I hate signed characters

      //
      //  lookup glyph index
      //

   info->glyph_index = FT_Get_Char_Index(face, (FT_ULong) k);

   if ( info->glyph_index == 0 )  {

      mlog << Error << "\n\n  CgraphBase::write_centered_text() -> glyph index not found!\n\n";

      exit ( 1 );

   }

      //
      //  load glyph
      //

   error = FT_Load_Glyph(face, info->glyph_index, FT_LOAD_DEFAULT);

   if ( error )  {

      mlog << Error << "\n\n  CgraphBase::write_centered_text() -> error loading glyph\n\n";

      exit ( 1 );

   }

      //
      //  get character metrics
      //

   metrics = &(face->glyph->metrics);

   x_bearing   = fix_to_float(metrics->horiBearingX);
   y_bearing   = fix_to_float(metrics->horiBearingY);

   advance     = fix_to_float(metrics->horiAdvance);

   char_width  = fix_to_float(metrics->width);
   char_height = fix_to_float(metrics->height);

      //
      //  get kern info, if any, for this char and the previous one
      //

   info->kern = 0.0;

   if ( use_kerning && !first_char )  {

      FT_Get_Kerning(face, previous, info->glyph_index, FT_KERNING_DEFAULT, &k_delta);

      info->kern = fix_to_float(k_delta.x);

      x_char_origin += info->kern;   //  move this char

   }

      //
      //  string bounding box Left, Right
      //

   if ( first_char )  L = x_bearing;

   if ( last_char )   R = x_char_origin + x_bearing + char_width;

      //
      //  string bounding box Top, Bottom
      //

   T = my_max(T, y_bearing);

   B = my_min(B, y_bearing - char_height);

      //
      //  character coordinates
      //

   info->x_outline_origin = x_char_origin;
   info->y_outline_origin = 0.0;

   info->x_bitmap_origin  = x_char_origin + x_bearing;
   info->y_bitmap_origin  = -y_bearing;

      //
      //  prepare for the next glyph (if any)
      //

   x_char_origin += advance;

   previous = info->glyph_index;

   first_char = false;

   ++info;

}   //  for j

LastTextWidth = R - L;

   //
   //  calculate the starting position on the page
   //

x_page = x_pin - u*(R - L);

if ( center == 1 )  y_page = y_pin;
else                y_page = y_pin + (B + v*(T - B));

   //
   //  adjust the character coordinates
   //

delta_x = x_page - L;
// delta_x = x_page;
delta_y = y_page;

info = wct_info;

for (j=0; j<N; ++j)  {

   info->x_bitmap_origin  += delta_x;
   info->y_bitmap_origin  += delta_y;

   info->x_outline_origin += delta_x;
   info->y_outline_origin += delta_y;

   info->y_outline_origin = c_fudge_y(info->y_outline_origin);   //  not sure why, but it works ...

   ++info;

}

   //
   //  render the text
   //

if ( render_flag )  {

   info = wct_info;

   for (j=0; j<N; ++j)  {

      error = FT_Load_Glyph(face, info->glyph_index, FT_LOAD_DEFAULT);

      if ( error )  {

         mlog << Error << "\n\n  CgraphBase::write_centered_text() -> error loading glyph\n\n";

         exit ( 1 );

      }

      switch ( fill_flag )  {


         case 0:
            draw_outline_ft_char (info->x_outline_origin, info->y_outline_origin, face->glyph);
            break;


         case 1:
            draw_bitmap_ft_char  (info->x_bitmap_origin, info->y_bitmap_origin, face->glyph);
            break;


         default:
            mlog << Error << "\n\n  CgraphBase::write_centered_text() -> fill_flag "
                 << fill_flag << " is not supported\n\n";
            exit ( 1 );
            break;


      }   //  switch

      ++info;

   }   //  for j

}   //  if render_flag

   //
   //  done
   //

if ( DoLigatures )  { delete [] new_string;  new_string = (char *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::curveto(double x1, double y1, double x2, double y2, double x3, double y3)

{

cairo_curve_to(Cr, x1, y1, x2, y2, x3, y3);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::line(double x1, double y1, double x2, double y2, bool newpath_flag)

{

if ( newpath_flag )  newpath();

moveto(x1, y1);
lineto(x2, y2);

if ( newpath_flag )  stroke();

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::arc (double xcen, double ycen, double radius, double angle_start, double angle_stop)

{

cairo_arc(Cr, xcen, ycen, radius, angle_start, angle_stop);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::arcn(double xcen, double ycen, double radius, double angle_start, double angle_stop)

{

cairo_arc_negative(Cr, xcen, ycen, radius, angle_start, angle_stop);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::import(const Ppm & ppm, double x, double y, double u, double v, double _scale)

{

import(ppm, x, y, u, v, _scale, _scale);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::import(const Ppm & ppm, double x, double y, double u, double v, double x_scale, double y_scale)

{

double x_ll, y_ll;
cairo_surface_t * i = (cairo_surface_t *) 0;
unsigned char * buf = (unsigned char *) 0;

gsave();

x_ll = x - x_scale*u*(ppm.nx());
y_ll = y - y_scale*(1.0 - v)*(ppm.ny());

translate(x_ll, y_ll);

scale(x_scale, y_scale);

i = cairo_image_surface_create (CAIRO_FORMAT_RGB24, ppm.nx(), ppm.ny());

buf = cairo_image_surface_get_data (i);

ppm.copy_data_32(buf, true);

cairo_surface_mark_dirty (i);


cairo_set_source_surface (Cr, i, 0.0, 0.0);

cairo_paint (Cr);

   //
   //  done
   //

cairo_surface_destroy (i);  i = (cairo_surface_t *) 0;

grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::draw_bitmap_ft_char(double x_bitmap_origin, double y_bitmap_origin, const FT_GlyphSlot slot)

{

int R, G, B, alpha;
int row, col, n;
int bytes_per_row;
int error;
cairo_surface_t * i = (cairo_surface_t *) 0;
unsigned char * buf = (unsigned char *) 0;
unsigned char * d = (unsigned char *) 0;
const Color cc = current_color();
const int IR = (int) (cc.red());
const int IG = (int) (cc.green());
const int IB = (int) (cc.blue());



error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

if ( error )  {

   mlog << Error << "\n\n  CgraphBase::draw_bitmap_ft_char() -> error rendering glyph\n\n";

   exit ( 1 );

}

const int W = slot->bitmap.width;
const int H = slot->bitmap.rows;

gsave();

// translate(x, y);

// i = cairo_image_surface_create (CAIRO_FORMAT_RGB24,  W, H);
   i = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, W, H);

buf = cairo_image_surface_get_data (i);


   //
   //  copy the glyph bitmap into the image surface buffer
   //

// bytes_per_row = 4*((W + 3)/4);
bytes_per_row = W;

d = buf;

for (row=0; row<H; ++row)  {

   for (col=0; col<W; ++col)  {

      n = row*bytes_per_row + col;

      alpha = (int) (slot->bitmap.buffer[n]);

         //
         //  according to the cairo API reference, 
         //     "pre-multiplied" alpha is used for 
         //     the format CAIRO_FORMAT_ARGB32
         //
         //     ( ... which I think is pretty stupid )
         //

      R = (IR*alpha)/255;
      G = (IG*alpha)/255;
      B = (IB*alpha)/255;

      *d++ = (unsigned char) B;
      *d++ = (unsigned char) G;
      *d++ = (unsigned char) R;

      *d++ = (unsigned char) alpha;

   }

}

   //
   //  render the bitmap
   //

cairo_surface_mark_dirty (i);

// cairo_set_source_surface (Cr, i, 100.0, 100.0);
   cairo_set_source_surface (Cr, i, x_bitmap_origin, y_bitmap_origin);

cairo_paint (Cr);

   //
   //  done
   //

cairo_surface_destroy (i);  i = (cairo_surface_t *) 0;

grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::draw_outline_ft_char(double x_ll, double y_ll, const FT_GlyphSlot slot)

{

if ( slot->format != FT_GLYPH_FORMAT_OUTLINE )  {

   mlog << Error << "\n\n  CgraphBase::draw_outline_ft_char() -> not an outline!\n\n";

   exit ( 1 );

}

FT_Outline * a = &(slot->outline);
FT_Outline_Funcs funcs;
FT_Error error = (FT_Error) 0;
ft_user_info user;


funcs.move_to  = &my_moveto;
funcs.line_to  = &my_lineto;
funcs.conic_to = &my_conic;
funcs.cubic_to = &my_cubic;

funcs.shift    = 0;
funcs.delta    = 0;

user.cgraph = this;

user.have_path = false;

user.x_offset = x_ll;
user.y_offset = y_ll;

newpath();

error = FT_Outline_Decompose(a, &funcs, &user);

if ( error )  {

   mlog << Error << "\n\n  CgraphBase::draw_outline_ft_char() -> error deomposing glyph!\n\n";

   exit ( 1 );

}

closepath();
stroke();




   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::fill(const Box & b, const Color & c)

{

gsave();

   set_color(c);

   newpath();

   moveto(b.left(),  b.bottom());
   lineto(b.right(), b.bottom());
   lineto(b.right(), b.top());
   lineto(b.left(),  b.top());

   closepath();

   fill();

grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::outline(const Box & b, const Color & c)

{

gsave();

   set_color(c);

   newpath();

   moveto(b.left(),  b.bottom());
   lineto(b.right(), b.bottom());
   lineto(b.right(), b.top());
   lineto(b.left(),  b.top());

   closepath();

   stroke();

grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::outline(const Box & b, const Color & c, const double linewidth)

{

gsave();

   set_color(c);

   setlinewidth(linewidth);

   newpath();

   moveto(b.left(),  b.bottom());
   lineto(b.right(), b.bottom());
   lineto(b.right(), b.top());
   lineto(b.left(),  b.top());

   closepath();

   stroke();

grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::setlinecap (int k)

{

switch ( k )  {

   case 0:
      setlinecap_butt();
      break;

   case 1:
      setlinecap_round();
      break;

   case 2:
      setlinecap_projecting_square();
      break;

   default:
      mlog << Error << "\n\n  CgraphBase::setlinecap(int) -> bad value ... " << k << "\n\n";
      exit ( 1 );
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::setlinejoin (int k)

{

switch ( k )  {

   case 0:
      setlinejoin_miter();
      break;

   case 1:
      setlinejoin_round();
      break;

   case 2:
      setlinejoin_bevel();
      break;

   default:
      mlog << Error << "\n\n  CgraphBase::setlinejoin(int) -> bad value ... " << k << "\n\n";
      exit ( 1 );
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


Color CgraphBase::current_color() const

{

Color c(0, 0, 0);

if ( CS.depth() > 0 )  c = CS.peek();

return ( c );

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::gsave()

{

cairo_save(Cr);

Color c = current_color();

CS.push(c);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::grestore()

{

cairo_restore(Cr);

Color c;

if ( CS.depth() > 0 )  c = CS.pop();

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::concat(const GeneralAffine & aff)

{

cairo_matrix_t M;

M.xx = aff.m11();
M.yy = aff.m22();

M.xy = aff.m12();
M.yx = aff.m21();

M.x0 = aff.tx();
M.y0 = aff.ty();

cairo_transform(Cr, &M);

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::device_to_user(double x_device, double y_device, double & x_user, double & y_user) const

{

double xx = x_device;
double yy = y_device;

cairo_device_to_user(Cr, &xx, &yy);

x_user = xx;
y_user = yy;

return;

}


////////////////////////////////////////////////////////////////////////


void CgraphBase::user_to_device(double x_user, double y_user, double & x_device, double & y_device) const

{

double xx = x_user;
double yy = y_user;

cairo_user_to_device(Cr, &xx, &yy);

x_device = xx;
y_device = yy;

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Cgraph
   //


////////////////////////////////////////////////////////////////////////


Cgraph::Cgraph()

{

cgraph2_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Cgraph::~Cgraph()

{

close();

}


////////////////////////////////////////////////////////////////////////


Cgraph::Cgraph(const Cgraph &)

{

mlog << Error << "\n\n  Cgraph::Cgraph(const Cgraph &) -> should never be called!\n\n";

exit ( 1 );

   // cgraph2_init_from_scratch();
   // 
   // assign(c);

}


////////////////////////////////////////////////////////////////////////


Cgraph & Cgraph::operator=(const Cgraph &)

{

mlog << Error << "\n\n  operator=Cgraph(const Cgraph &) -> should never be called!\n\n";

exit ( 1 );


// if ( this == &c )  return ( * this );
// 
// assign(c);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Cgraph::cgraph2_init_from_scratch()

{

cgraph_init_from_scratch();

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::write_centered_text(int center, int fill_flag, double x_pin, double y_pin, double u, double v, const char c, const bool render_flag)

{

char junk[2];

junk[0] = c;
junk[1] = (char) 0;

CgraphBase::write_centered_text(center, fill_flag, x_pin, MediaHeight - y_pin, u, v, junk, render_flag);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::write_centered_text(int center, int fill_flag, double x_pin, double y_pin, double u, double v, const char * text, const bool render_flag)

{

CgraphBase::write_centered_text(center, fill_flag, x_pin, MediaHeight - y_pin, u, v, text, render_flag);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::curveto(double x1, double y1, double x2, double y2, double x3, double y3)

{

CgraphBase::curveto(x1, c_fudge_y(y1), x2, c_fudge_y(y2), x3, c_fudge_y(y3));

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::import(const Ppm & ppm, const Box & b, const ViewGravity g)

{

double u, v, x, y, x_scale, y_scale;


if ( g == fill_viewport )  {

   x = b.left();
   y = b.bottom();

   u = v = 0.0;

   x_scale = (b.width())/(ppm.nx());

   y_scale = (b.height())/(ppm.ny());

} else {

   viewgravity_to_uv(g, u, v);

   x = b.left()   + u*(b.width());
   y = b.bottom() + v*(b.height());

   x_scale = y_scale = calc_mag(ppm.nx(), ppm.ny(), b.width(), b.height());

}

import(ppm, x, y, u, v, x_scale, y_scale);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::import(const Ppm & ppm, double x, double y, double u, double v, double _scale)

{

import(ppm, x, y, u, v, _scale, _scale);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::import(const Ppm & ppm, double x, double y, double u, double v, double x_scale, double y_scale)

{

double x_ll, y_ul, y_ll;
cairo_surface_t * i = (cairo_surface_t *) 0;
unsigned char * buf = (unsigned char *) 0;
const double scaled_nx = x_scale*(ppm.nx());
const double scaled_ny = y_scale*(ppm.ny());


gsave();


x_ll = x - u*scaled_nx;

y_ll = y - v*scaled_ny;

y_ul = y_ll + scaled_ny;


cairo_translate(Cr, x_ll, c_fudge_y(y_ul));

cairo_scale(Cr, x_scale, y_scale);


i = cairo_image_surface_create (CAIRO_FORMAT_RGB24, ppm.nx(), ppm.ny());

buf = cairo_image_surface_get_data (i);

ppm.copy_data_32(buf, true);

cairo_surface_mark_dirty (i);


cairo_set_source_surface (Cr, i, 0.0, 0.0);

cairo_paint (Cr);


   //
   //  done
   //

cairo_surface_destroy (i);  i = (cairo_surface_t *) 0;

grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::concat(const GeneralAffine & aff)

{

cairo_matrix_t M;
Affine A, F, FI, result;



A.set_mb(aff.m11(), aff.m12(), aff.m21(), aff.m22(), aff.tx(), aff.ty());

F.set_mb(1.0, 0.0, 0.0, -1.0, 0.0, MediaHeight);

FI = F;

FI.invert();

   result = F*A*FI;
// result = FI*A*F;
// result = A*F;
// result = F*A;
// result = A;


M.xx = result.m11();
M.yy = result.m22();

M.xy = result.m12();
M.yx = result.m21();

M.x0 = result.tx();
M.y0 = result.ty();

cairo_transform(Cr, &M);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::translate(double dx, double dy)

{

Affine a;

a.set_mb(1.0, 0.0, 0.0, 1.0, dx, dy);

concat(a);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::scale(double s)

{

Affine a;

a.set_mb(s, 0.0, 0.0, s, 0.0, 0.0);

concat(a);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::scale(double sx, double sy)

{

Affine a;

a.set_mb(sx, 0.0, 0.0, sy, 0.0, 0.0);

concat(a);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::rotate(double angle)

{

const double S = sind(angle);
const double C = cosd(angle);

Affine a;

a.set_mb(C, -S, S, C, 0.0, 0.0);

concat(a);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::arc (double xcen, double ycen, double radius, double angle_start, double angle_stop)

{

cairo_arc(Cr, xcen, c_fudge_y(ycen), radius, angle_start, angle_stop);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::arcn(double xcen, double ycen, double radius, double angle_start, double angle_stop)

{

cairo_arc_negative(Cr, xcen, c_fudge_y(ycen), radius, angle_start, angle_stop);

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::clip(const Box & b)

{

newpath();

moveto(b.left(),  b.bottom());
lineto(b.right(), b.bottom());
lineto(b.right(), b.top());
lineto(b.left(),  b.top());

closepath();

CgraphBase::clip();

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::clip()

{

CgraphBase::clip();

return;

}


////////////////////////////////////////////////////////////////////////


void Cgraph::set_background(const Color & c)

{

fill(PageBox, c);

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void dump_metrics(const FT_Glyph_Metrics & m)

{

   //
   //  values are in 26.6 format
   //

const double t = 1.0/64.0;   //  2^(-6)

cout << "\n\n";

cout << "width        " << t*(m.width)  << '\n';
cout << "height       " << t*(m.height) << '\n';

cout << '\n';

cout << "horiBearingX " << t*(m.horiBearingX) << '\n';
cout << "horiBearingY " << t*(m.horiBearingY) << '\n';
cout << "horiAdvance  " << t*(m.horiAdvance)  << '\n';

cout << "\n\n";


return;

}


////////////////////////////////////////////////////////////////////////


void remap_string(FT_Face face, const char * in, const char * & out)

{

if ( empty(in) )  {

   mlog << Error << "\n\n  remap_string(FT_Face, const char * in, const char * & out) -> empty string!\n\n";

   exit ( 1 );

}

int j, k;
char c0, c1;
const int N = strlen(in);
char * s = (char *) 0;
FT_UInt fi_glyph_index = 0;
FT_UInt fl_glyph_index = 0;

s = new char [N + 1];

memset(s, 0, N + 1);

fi_glyph_index = FT_Get_Char_Index(face, (FT_ULong) fi_ligature);
fl_glyph_index = FT_Get_Char_Index(face, (FT_ULong) fl_ligature);

j = k = 0;

while ( j < N )  {

   c0 = in[j];

   if ( (j + 1) >= N )  c1 = 0;
   else                 c1 = in[j + 1];

   if ( (c0 == 'f') && (c1 == 'i') && fi_glyph_index )  {

      s[k++] = (char) fi_ligature;

      j += 2;

   } else if ( (c0 == 'f') && (c1 == 'l') && fl_glyph_index )  {

      s[k++] = (char) fl_ligature;

      j += 2;

   } else {

      s[k++] = c0;

      ++j;

   }

}   //  while

   //
   //  done
   //

out = s;  s = (char *) 0;

return;

}


////////////////////////////////////////////////////////////////////////


int my_moveto (const FT_Vector * to, void * u)

{

ft_user_info * info = (ft_user_info *) u;

if ( info->have_path )  info->cgraph->closepath();

double x, y;

x = (info->x_offset) + fix_to_float(to->x);
y = (info->y_offset) + fix_to_float(to->y);

info->cgraph->moveto(x, y);

info->have_path = true;

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int my_lineto (const FT_Vector * to, void * u)

{

ft_user_info * info = (ft_user_info *) u;

double x, y;

x = (info->x_offset) + fix_to_float(to->x);
y = (info->y_offset) + fix_to_float(to->y);

info->cgraph->lineto(x, y);

info->have_path = true;

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int my_conic (const FT_Vector * control, const FT_Vector * to, void * u)

{

ft_user_info * info = (ft_user_info *) u;

mlog << Error << "\n\n  my_conic() -> should never be called!\n\n";

exit ( 1 );

info->have_path = true;

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


int my_cubic (const FT_Vector * control1, const FT_Vector * control2, const FT_Vector * to, void * u)

{

ft_user_info * info = (ft_user_info *) u;

double x1, y1, x2, y2, x3, y3;

x1 = (info->x_offset) + fix_to_float(control1->x);
y1 = (info->y_offset) + fix_to_float(control1->y);

x2 = (info->x_offset) + fix_to_float(control2->x);
y2 = (info->y_offset) + fix_to_float(control2->y);

x3 = (info->x_offset) + fix_to_float(to->x);
y3 = (info->y_offset) + fix_to_float(to->y);

info->cgraph->curveto(x1, y1, x2, y2, x3, y3);

info->have_path = true;

return ( 0 );

}


////////////////////////////////////////////////////////////////////////




