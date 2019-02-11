// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_MODE_PS_FILE_H__
#define  __MET_MODE_PS_FILE_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"
#include "vx_ps.h"
#include "engine.h"
#include "vx_grid.h"


////////////////////////////////////////////////////////////////////////


static const double L_thin  = 0.50;
static const double L_thick = 1.00;


static const bool use_zlib = true;


////////////////////////////////////////////////////////////////////////


enum EngineType {
   NoEng = 0,
   FOEng = 1,
   FFEng = 2,
   OOEng = 3
};


////////////////////////////////////////////////////////////////////////


class ModePsFile : public PSfile {

   private:

      ModePsFile(const ModePsFile &);
      ModePsFile & operator=(const ModePsFile &);

   protected:

      void mpsf_init_from_scratch();

      void mpsf_assign(const ModePsFile &);


      void set_xy_box();

      void set_postscript_dims();

      void set_view(double y_ll, double y_ur, double x_cen);


      void plot_engine            (ModeFuzzyEngine &, EngineType, const char * title);
      void plot_threshold_merging (ModeFuzzyEngine &, const char * title , bool fcst);

      void do_page_1       (ModeFuzzyEngine &, EngineType, const char * title);
      void do_page_1_FOEng (ModeFuzzyEngine &, EngineType, const char * title);
      void do_page_1_other (ModeFuzzyEngine &, EngineType, const char * title);

      void do_fcst_enlarge_page (ModeFuzzyEngine &, EngineType, const char * title);
      void do_obs_enlarge_page  (ModeFuzzyEngine &, EngineType, const char * title);
      void do_overlap_page      (ModeFuzzyEngine &, EngineType, const char * title);
      void do_cluster_page      (ModeFuzzyEngine &, EngineType, const char * title);

      void draw_boundaries(ModeFuzzyEngine &, bool fcst);
      void draw_boundaries(ShapeData &, int n_shapes);

      void draw_polyline(Polyline &, const Color &, bool latlon);

      void draw_convex_hulls(ModeFuzzyEngine & eng, bool fcst, bool id_flag);

      void plot_simple_ids(ModeFuzzyEngine &, bool fcst);

      void render_ppm(ModeFuzzyEngine &, EngineType, const ShapeData &, bool fcst, int split);

      void draw_colorbar(bool);

      void draw_map(MetConfig *);

      void wct0(double x, double y, const char *);
      void wct5(double x, double y, const char *);


      ModeFuzzyEngine  * Engine;     //  not allocated
      ModeConfInfo     * ConfInfo;   //  not allocated
      const Grid       * grid;       //  not allocated

      ConcatString MetDataDir;

      ConcatString FcstString;
      ConcatString  ObsString;

      ConcatString FcstShortString;
      ConcatString  ObsShortString;

      ColorTable   FcstRawCtable;
      ColorTable   ObsRawCtable;

      Color FcstFillColor;
      Color  ObsFillColor;

      double PageWidth;
      double PageHeight;

      double Hmargin;
      double Vmargin;

      double TextSep;

      double text_y;

      double DataMin;
      double DataMax;

      double Htab_1;
      double Htab_2;
      double Htab_3;

      double Vtab_1;
      double Vtab_2;
      double Vtab_3;

      double SmallPlotHeight;
      double LargePlotHeight;

      Box LargePane;
      Box SmallPane;

      Box XY_box;
      Box View_box;

   public:

      ModePsFile();
      ModePsFile(const char *);
     ~ModePsFile();

         //
         //  set stuff
         //

      void set (ModeFuzzyEngine &, const Grid &, double data_min, double data_max);

         //
         //  get stuff
         //


         //
         //  do stuff
         //

      void make_plot();

      using PSfile::outline_box;

      void outline_box (const Box &, const double linewidth);
      void outline_view();
      void fill_box    (const Box &, const Color &);


      void gridxy_to_pagexy(double x_grid, double y_grid, double & x_page, double & y_page) const;

      void set_color(const Color &);

      void choose_font(int, double);   //  uses MetDataDir

      void nextline();


};


////////////////////////////////////////////////////////////////////////


inline void ModePsFile::nextline() { text_y -= TextSep;  return; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_MODE_PS_FILE_H__  */


////////////////////////////////////////////////////////////////////////


