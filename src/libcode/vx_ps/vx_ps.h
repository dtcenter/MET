// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



//////////////////////////////////////////////////////////////


#ifndef  __VERIF_POSTSCRIPT_H__
#define  __VERIF_POSTSCRIPT_H__


//////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>

#include "afm.h"
#include "vx_ps_text.h"


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


struct Ps_Bbox {

   double left;
   double right;
   double top;
   double bottom;

};


//////////////////////////////////////////////////////////////


class PSfile {

   protected:

      void init_from_scratch();

      int current_font;
      double font_size;

      PSfile(const PSfile &);
      PSfile & operator=(const PSfile &);

      void write_single_node(const VxpsTextNode *, double x_cur, double y_cur, int fill_flag);

      ofstream * File;   //  the output file

      char * eps_output_filename;

      void finish_eps();

   public:

      int is_eps;

      int bbox_x_ll;
      int bbox_y_ll;
      int bbox_x_ur;
      int bbox_y_ur;


      ofstream & file() const;

      Afm * afm;   //  allocated

      PSfile();
      virtual ~PSfile();

      virtual void open(const char *);
      virtual void eps_open(const char *);
      virtual void close();

      virtual void choose_font(int, double);

      virtual void choose_font(int, double, const char *);

      virtual void write_centered_text(int, int, double, double, double, double, const char *);

      virtual void newpath();

      virtual void moveto(double, double);
      virtual void lineto(double, double);

      virtual void translate (double, double);
      virtual void rotate    (double);

      virtual void fill();
      virtual void clip();
      virtual void stroke();
      virtual void closepath();

      virtual void line(double, double, double, double, int);

      virtual void setgray(double);
      virtual void setlinewidth(double);

      virtual void showpage();

      virtual void pagenumber(int);

      virtual void box(double, double, double, double, double);

      virtual void circle(double cx, double cy, double radius, int stroke_flag);

      virtual void setrgbcolor(double, double, double);
      virtual void sethsbcolor(double, double, double);

      virtual void gsave();
      virtual void grestore();

};


//////////////////////////////////////////////////////////////


inline ofstream & PSfile::file() const { return ( *File ); }


//////////////////////////////////////////////////////////////


extern Ps_Bbox lo_res_bbox(const char *);
extern Ps_Bbox hi_res_bbox(const char *);


//////////////////////////////////////////////////////////////


extern void make_list(const int font_number, const double font_size, const Afm & afm,
                      VxpsTextNode & node, const char * s);


//////////////////////////////////////////////////////////////


#endif   //  __VERIF_POSTSCRIPT_H__


//////////////////////////////////////////////////////////////




