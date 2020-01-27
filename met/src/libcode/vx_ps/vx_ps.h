// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



//////////////////////////////////////////////////////////////


#ifndef  __VX_PS_H__
#define  __VX_PS_H__


//////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>

#include "afm.h"
#include "ps_text.h"

#include "psout_filter.h"

#include "vx_color.h"

#include "affine.h"


//////////////////////////////////////////////////////////////


   //
   //  total number of fonts
   //

static const int total_vx_ps_fonts = 34;


//////////////////////////////////////////////////////////////


   //
   //  path relative to DATA_DIR
   //

static const char afm_directory [] = "ps";


//////////////////////////////////////////////////////////////


static const char papersize_env [] = "MET_PAPER_SIZE";


//////////////////////////////////////////////////////////////


enum DocumentMedia {

   MediaLetter, 
   MediaA4, 

   no_document_media, 

};


//////////////////////////////////////////////////////////////


enum DocumentOrientation {

   OrientationPortrait, 
   OrientationLandscape, 

   no_document_orientation

};


//////////////////////////////////////////////////////////////


enum FontFamily {

   ff_Helvetica,
   ff_NewCentury,
   ff_Palatino,
   ff_Times,
   ff_Courier,
   ff_Bookman,

   no_font_family

};


//////////////////////////////////////////////////////////////


class PSfile {

   protected:

      void init_from_scratch();


      PSfile(const PSfile &);
      PSfile & operator=(const PSfile &);

      void write_single_node(const VxpsTextNode *, double x_cur, double y_cur, int fill_flag);

      void do_prolog();

      void set_media       (DocumentMedia);
      void set_orientation (DocumentOrientation);

      int current_font;
      double font_size;

      DocumentOrientation Orientation;
      DocumentMedia       Media;

      ofstream * File;   //  the output file ... allocated

      ConcatString OutputFilename;

      Afm * afm;   //  allocated

      FontFamily Family;   //  defaults to ff_Times

      int showpage_count;

      

         //
         //  These do NOT account for document orientation
         //
      double MediaWidth;    //  points
      double MediaHeight;   //  points

   public:

      PSfile();
      virtual ~PSfile();

      PSOutputFilter psout;

      PSFilter * Head;

         //
         //  set stuff
         //

      virtual void choose_font_with_dir(int, double, const char *);

         //
         //  get stuff
         //

      // ofstream & file() const;
      PSFilter & file() const;

      DocumentOrientation orientation () const;
      DocumentMedia       media       () const;

      ConcatString output_filename() const;

      double page_width  () const;
      double page_height () const;

      bool is_portrait  () const;
      bool is_landscape () const;

      int pagenumber() const;

      FontFamily family () const;

         //
         //  do stuff
         //

      virtual void open(const char *);   //  uses default media and portrait orientation
      virtual void open(const char *, DocumentOrientation);   //  uses default media
      virtual void open(const char *, DocumentMedia, DocumentOrientation);
      virtual void close();


      virtual void write_centered_text(int, int, double, double, double, double, const char *, const bool render_flag = true);

      virtual void showpage();

      virtual void pagenumber(int);
      virtual void inc_pagenumber();   //  increment page number

      virtual void gsave();
      virtual void grestore();

      virtual void comment(const char * text);

         //
         //  fonts
         //

      virtual void choose_font(int, double);

      virtual void  set_family     (FontFamily);
      virtual void  set_font_size  (double);

      virtual void  roman      (double size);
      virtual void  italic     (double size);
      virtual void  bold       (double size);
      virtual void  bolditalic (double size);

      virtual void  dingbats   (double size);   //  ZapfDingbats font (# 33)
      virtual void  symbol     (double size);   //  Symbol font       (# 27)

      virtual void  roman      ();
      virtual void  italic     ();
      virtual void  bold       ();
      virtual void  bolditalic ();

      virtual int roman_font      () const;
      virtual int italic_font     () const;
      virtual int bold_font       () const;
      virtual int bolditalic_font () const;

         //
         //  graphics
         //

      virtual void newpath();

      virtual void moveto(double, double);
      virtual void lineto(double, double);

      virtual void translate (double, double);
      virtual void rotate    (double);

      virtual void fill();
      virtual void clip();
      virtual void stroke();
      virtual void closepath();

      virtual void line(double, double, double, double, bool do_newpath = true);

      virtual void setgray(double);
      virtual void setlinewidth(double);
      virtual void setdash(const char *);

      virtual void box(double, double, double, double, double);

      virtual void circle(double cx, double cy, double radius, bool stroke_flag = true);

      virtual void setrgbcolor(double, double, double);
      virtual void sethsbcolor(double, double, double);

      virtual void set_color(const Color &);

      virtual void outline_box (const Box &);
      virtual void outline_box (const Box &, const double linewidth);
      virtual void outline_box (const Box &, const Color &, double linewidth);

      virtual void fill_box (const Box &, const Color &);

      virtual void setlinecap  (int);
      virtual void setlinecap_butt();              // 0)
      virtual void setlinecap_round();             // 1)
      virtual void setlinecap_projecting_square(); // 2)

      virtual void setlinejoin (int);
      virtual void setlinejoin_miter();   //  0
      virtual void setlinejoin_round();   //  1
      virtual void setlinejoin_bevel();   //  2

         //
         //  zlib compression stuff
         //

      PSFilter * fa_bank;   //  allocated

      virtual void begin_flate();
      virtual void end_flate();

};


//////////////////////////////////////////////////////////////


inline PSFilter & PSfile::file() const { return ( *Head ); }

inline DocumentOrientation PSfile::orientation () const { return ( Orientation ); }
inline DocumentMedia       PSfile::media       () const { return ( Media ); }

inline ConcatString PSfile::output_filename () const { return ( OutputFilename ); }

inline double PSfile::page_width  () const { return ( (Orientation == OrientationPortrait) ? MediaWidth  : MediaHeight); }
inline double PSfile::page_height () const { return ( (Orientation == OrientationPortrait) ? MediaHeight : MediaWidth ); }

inline bool PSfile::is_portrait  () const { return ( Orientation == OrientationPortrait  ); }
inline bool PSfile::is_landscape () const { return ( Orientation == OrientationLandscape ); }

inline int PSfile::pagenumber() const { return ( showpage_count + 1); }

inline void PSfile::inc_pagenumber() { pagenumber(showpage_count + 1);  return; }

inline void PSfile::setlinecap_butt()              { setlinecap(0);  return; };
inline void PSfile::setlinecap_round()             { setlinecap(1);  return; };
inline void PSfile::setlinecap_projecting_square() { setlinecap(2);  return; };

inline void PSfile::setlinejoin_miter() { setlinejoin(0);  return; };
inline void PSfile::setlinejoin_round() { setlinejoin(1);  return; };
inline void PSfile::setlinejoin_bevel() { setlinejoin(2);  return; };

inline FontFamily PSfile::family () const { return ( Family ); }


//////////////////////////////////////////////////////////////


extern void make_list(const int font_number, const double font_size, const Afm & afm,
                      VxpsTextNode & node, const char * s);


extern int ff_to_roman      (const FontFamily);
extern int ff_to_italic     (const FontFamily);
extern int ff_to_bold       (const FontFamily);
extern int ff_to_bolditalic (const FontFamily);


//////////////////////////////////////////////////////////////


#endif   //  __VX_PS_H__


//////////////////////////////////////////////////////////////




