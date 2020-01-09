// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MY_CAIRO_GRAPHICS_MAIN_H__
#define  __MY_CAIRO_GRAPHICS_MAIN_H__


////////////////////////////////////////////////////////////////////////


#include "vx_ps.h"
#include "vx_pxm.h"
#include "vx_math.h"

#include "color_stack.h"
#include "cgraph_font.h"

#include "cairo.h"
#include "cairo-svg.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_ERRORS_H


////////////////////////////////////////////////////////////////////////


   //
   //  Don't use this class.
   //
   //  Use the Cgraph class derived from this.
   //

class CgraphBase : public PSfile {

   public:

      enum PlotType {

          png_type, 
          ps_type, 
          eps_type, 
          svg_type, 
          buffer_type, 

          no_cgraph_plot_type

      };

   private:

      CgraphBase(const CgraphBase &);
      CgraphBase & operator=(const CgraphBase &);

      void finish();   //  does nothing

   // protected:
   public:

      void cgraph_init_from_scratch();

      // void assign(const CgraphBase &);


      void write();

      

      // double image_to_ps(double);   //  image y coordinate to postscript y

      void draw_bitmap_ft_char  (double x_ul, double y_ul, const FT_GlyphSlot);
      void draw_outline_ft_char (double x_ll, double y_ll, const FT_GlyphSlot);

      FT_Face ft_load(const char * pfb_path, const char * afm_path);   //  does the freetype stuff

      void load_ps_font  (int);

      void set_current_font_size(double);

      double current_font_size() const;

      ConcatString Filename;


      cairo_surface_t * Surface;   //  allocated through the cairo library

      cairo_t * Cr;                //  allocated through the cairo library

      PlotType Ptype;

      Box PageBox;

      bool DoKerning;     //  default: true
      bool DoLigatures;   //  default: true

      ColorStack CS;   //  cairo doesn't let us retrieve the current color, so
                       //  we have to keep track of it ourselves.
                       //  current color is needed for rendering text


      FT_Library Library;

      CgFontCollection Fonts;

      CgFont * CurrentFont;   //  not allocated

      double LastTextWidth;


   public:

      CgraphBase();
      virtual ~CgraphBase();

      using PSfile::open;

      void open(const char * filename, int width, int height, PlotType = CgraphBase::png_type);

      void close();

      void showpage();

      void pagenumber(int);   //  does nothing

      void comment(const char *);   //  does nothing

      void set_do_kerning   (bool);
      void set_do_ligatures (bool);

      virtual const Box & page() const;

      virtual Color current_color() const;

      virtual double c_fudge_y(double) const;

         //
         //  import images
         //

      virtual void import(const Ppm &, double x, double y, double u, double v, double _scale = 1.0);
      virtual void import(const Ppm &, double x, double y, double u, double v, double x_scale, double y_scale);

         //  this is implemented in the CGraph2 class
      // virtual void import(const Ppm &, const Box &, const ViewGravity);

         //
         //  Type 1 fonts
         //

      void choose_font(int, double);                     //  from ps base class


      virtual void write_centered_text(int center, int fill_flag, double x_pin, double y_pin, double u, double v, const char,   const bool render_flag = true);
      virtual void write_centered_text(int center, int fill_flag, double x_pin, double y_pin, double u, double v, const char *, const bool render_flag = true);

         //
         //  text rendering, with the cairo "toy" interface
         //

      // void choose_font(const char * name, FontWeight, FontSlant, double size);
      // void choose_font(const char * name, FontWeight, double size);   //  unslanted
      // 
      // virtual void wct(double x, double y, double u, double v, const char *);

         //
         //  drawing
         //

      void set_color(const Color &);
      void setrgbcolor(double, double, double);
      void setgray(double);

      void setlinewidth(double);

      virtual void setlinecap  (int);
      virtual void setlinecap_butt();              // 0
      virtual void setlinecap_round();             // 1
      virtual void setlinecap_projecting_square(); // 2

      virtual void setlinejoin (int);
      virtual void setlinejoin_miter();   //  0
      virtual void setlinejoin_round();   //  1
      virtual void setlinejoin_bevel();   //  2


      void newpath();

      void moveto(double, double);
      void lineto(double, double);

      virtual void curveto(double, double, double, double, double, double);

      void rmoveto(double, double);
      void rlineto(double, double);

      void translate (double, double);
      void rotate    (double);          //  degrees
      void scale     (double);
      void scale     (double, double);

      void concat(const GeneralAffine &);

      void device_to_user(double, double, double &, double &) const;
      void user_to_device(double, double, double &, double &) const;

      void closepath();
      void stroke();
      void fill();
      void clip();

      void gsave();
      void grestore();

      void line(double, double, double, double, bool = true);

      void arc  (double xcen, double ycen, double radius, double angle_start, double angle_stop);
      void arcn (double xcen, double ycen, double radius, double angle_start, double angle_stop);

      void fill(const Box &, const Color &);

      void outline(const Box &, const Color &);
      void outline(const Box &, const Color &, const double linewidth);

};


////////////////////////////////////////////////////////////////////////


inline void CgraphBase::finish() { return; }

inline void CgraphBase::showpage() { close();  return; }

inline void CgraphBase::pagenumber(int) { return; }

inline void CgraphBase::comment(const char *) { return; }

inline void CgraphBase::setlinewidth(double __W__) { cairo_set_line_width(Cr, __W__);  return; }

inline void CgraphBase::moveto(double __X__, double __Y__) { cairo_move_to(Cr, __X__, __Y__);  return; }
inline void CgraphBase::lineto(double __X__, double __Y__) { cairo_line_to(Cr, __X__, __Y__);  return; }

inline void CgraphBase::rmoveto(double __X__, double __Y__) { cairo_rel_move_to(Cr, __X__, __Y__);  return; }
inline void CgraphBase::rlineto(double __X__, double __Y__) { cairo_rel_line_to(Cr, __X__, __Y__);  return; }

inline void CgraphBase::rotate   (double __X__) { cairo_rotate(Cr, rad_per_deg * __X__);  return; }
inline void CgraphBase::translate(double __X__, double __Y__) { cairo_translate(Cr, __X__, __Y__);  return; }

inline void CgraphBase::scale(double __X__, double __Y__) { cairo_scale(Cr, __X__, __Y__);  return; }
inline void CgraphBase::scale(double __X__)               { cairo_scale(Cr, __X__, __X__);  return; }

inline void CgraphBase::newpath()   { cairo_new_path   (Cr);  return; }
inline void CgraphBase::closepath() { cairo_close_path (Cr);  return; }
inline void CgraphBase::stroke()    { cairo_stroke     (Cr);  return; }
inline void CgraphBase::fill()      { cairo_fill       (Cr);  return; }
inline void CgraphBase::clip()      { cairo_clip       (Cr);  return; }

inline const Box & CgraphBase::page() const { return ( PageBox ); }

inline void CgraphBase::setlinecap_butt()              { cairo_set_line_cap (Cr, CAIRO_LINE_CAP_BUTT);    return; }
inline void CgraphBase::setlinecap_round()             { cairo_set_line_cap (Cr, CAIRO_LINE_CAP_ROUND);   return; }
inline void CgraphBase::setlinecap_projecting_square() { cairo_set_line_cap (Cr, CAIRO_LINE_CAP_SQUARE);  return; }

inline void CgraphBase::setlinejoin_miter() { cairo_set_line_join (Cr, CAIRO_LINE_JOIN_MITER);  return; }
inline void CgraphBase::setlinejoin_round() { cairo_set_line_join (Cr, CAIRO_LINE_JOIN_ROUND);  return; }
inline void CgraphBase::setlinejoin_bevel() { cairo_set_line_join (Cr, CAIRO_LINE_JOIN_BEVEL);  return; }

inline double CgraphBase::c_fudge_y(double __Y__) const { return ( __Y__ ); }


////////////////////////////////////////////////////////////////////////


class Cgraph : public CgraphBase {

   private:

      Cgraph(const Cgraph &);
      Cgraph & operator=(const Cgraph &);

   public:

      void cgraph2_init_from_scratch();

      // void assign(const Cgraph &);


      double c_fudge_y(double) const;   //  image y coordinate to postscript y


   public:

      Cgraph();
      virtual ~Cgraph();

         //
         //  import images
         //

      void import(const Ppm &, double x, double y, double u, double v, double _scale = 1.0);
      void import(const Ppm &, double x, double y, double u, double v, double x_scale, double y_scale);
      void import(const Ppm &, const Box &, const ViewGravity);

         //
         //  text rendering
         //

      void write_centered_text(int center, int fill_flag, double x_pin, double y_pin, double u, double v, const char,   const bool render_flag = true);
      void write_centered_text(int center, int fill_flag, double x_pin, double y_pin, double u, double v, const char *, const bool render_flag = true);

         //
         //  drawing utils
         //

      void set_background(const Color &);

      void moveto(double, double);
      void lineto(double, double);

      void rmoveto(double, double);
      void rlineto(double, double);

      void curveto(double, double, double, double, double, double);

      void translate (double, double);
      void rotate    (double);          //  degrees
      void scale     (double);
      void scale     (double, double);

      void concat(const GeneralAffine &);   //  works!

      void clip(const Box &);
      void clip();

      void arc  (double xcen, double ycen, double radius, double angle_start, double angle_stop);
      void arcn (double xcen, double ycen, double radius, double angle_start, double angle_stop);

};


////////////////////////////////////////////////////////////////////////


inline double Cgraph::c_fudge_y(double __Y__) const { return ( MediaHeight - __Y__); }

inline void Cgraph::moveto(double __X__, double __Y__) { cairo_move_to(Cr, __X__, c_fudge_y(__Y__));  return; }
inline void Cgraph::lineto(double __X__, double __Y__) { cairo_line_to(Cr, __X__, c_fudge_y(__Y__));  return; }

inline void Cgraph::rmoveto(double __X__, double __Y__) { cairo_rel_move_to(Cr, __X__, -__Y__);  return; }
inline void Cgraph::rlineto(double __X__, double __Y__) { cairo_rel_line_to(Cr, __X__, -__Y__);  return; }

// inline void Cgraph::translate(double __X__, double __Y__) { cairo_translate(Cr, __X__, -__Y__);  return; }


////////////////////////////////////////////////////////////////////////


extern void dump_metrics(const FT_Glyph_Metrics &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MY_CAIRO_GRAPHICS_MAIN_H__  */


////////////////////////////////////////////////////////////////////////


