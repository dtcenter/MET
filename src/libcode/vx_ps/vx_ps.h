// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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

   no_document_orientation, 

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

      int showpage_count;

         //
         //  These do NOT account for document orientation
         //
      double MediaWidth;    //  points
      double MediaHeight;   //  points

   public:

      PSfile();
      virtual ~PSfile();

         //
         //  set stuff
         //

      virtual void choose_font(int, double, const char *);

         //
         //  get stuff
         //

      ofstream & file() const;

      DocumentOrientation orientation () const;
      DocumentMedia       media       () const;

      ConcatString output_filename() const;

      double page_width  () const;
      double page_height () const;

      bool is_portrait  () const;
      bool is_landscape () const;

         //
         //  do stuff
         //

      virtual void open(const char *);   //  uses default media and portrait orientation
      virtual void open(const char *, DocumentOrientation);   //  uses default media
      virtual void open(const char *, DocumentMedia, DocumentOrientation);
      virtual void close();

      virtual void choose_font(int, double);

      virtual void write_centered_text(int, int, double, double, double, double, const char *);

      virtual void showpage();

      virtual void pagenumber(int);

      virtual void gsave();
      virtual void grestore();

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

      virtual void box(double, double, double, double, double);

      virtual void circle(double cx, double cy, double radius, bool stroke_flag = true);

      virtual void setrgbcolor(double, double, double);
      virtual void sethsbcolor(double, double, double);

};


//////////////////////////////////////////////////////////////


inline ofstream & PSfile::file() const { return ( *File ); }

inline DocumentOrientation PSfile::orientation () const { return ( Orientation ); }
inline DocumentMedia       PSfile::media       () const { return ( Media ); }

inline ConcatString PSfile::output_filename () const { return ( OutputFilename ); }

inline double PSfile::page_width  () const { return ( (Orientation == OrientationPortrait) ? MediaWidth  : MediaHeight); }
inline double PSfile::page_height () const { return ( (Orientation == OrientationPortrait) ? MediaHeight : MediaWidth ); }

inline bool PSfile::is_portrait  () const { return ( Orientation == OrientationPortrait  ); }
inline bool PSfile::is_landscape () const { return ( Orientation == OrientationLandscape ); }


//////////////////////////////////////////////////////////////


extern void make_list(const int font_number, const double font_size, const Afm & afm,
                      VxpsTextNode & node, const char * s);


//////////////////////////////////////////////////////////////


#endif   //  __VX_PS_H__


//////////////////////////////////////////////////////////////




