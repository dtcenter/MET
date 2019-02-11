// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __PS_TABLE_HELPER_H__
#define  __PS_TABLE_HELPER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_ps.h"


////////////////////////////////////////////////////////////////////////


class TableHelper {

   protected:

      TableHelper(const TableHelper &);
      TableHelper  & operator=(const TableHelper &);

      void init_from_scratch();

      void assign(const TableHelper &);

      void range_check_rc(int r, int c) const;

      void range_check_r(int r) const;
      void range_check_c(int c) const;

      void check_plot() const;


      int Nrows;
      int Ncols;

      double Xpin;
      double Ypin;
      double Upin;
      double Vpin;

      bool PinIsSet;

      double * ColWidth;     //  allocated

      double * RowHeight;    //  allocated

      PSfile * Plot;   //  not allocated


   public:

      TableHelper();
     ~TableHelper();

      void clear();

         //
         //  set stuff
         //

      void set(PSfile &, int _nrows, int _ncols);

      void set_cell_wh(double, double);

      void set_col_width  (int c, double);
      void set_row_height (int r, double);

      void set_pin(double x, double y, double u, double v);

         //
         //  get stuff
         //

      int nrows() const;
      int ncols() const;

      double row_top    (int r) const;
      double row_bottom (int r) const;

      double col_left   (int c) const;
      double col_right  (int c) const;

      double col_width  (int c) const;
      double row_height (int r) const;

      double width  () const;   //  whole table
      double height () const;   //  whole table

      double left   () const;
      double right  () const;
      double top    () const;
      double bottom () const;

      void cell_ll  (int r, int c, double & x_ll, double & y_ll) const;
      void cell_wh  (int r, int c, double & w, double & h) const;

      void table_ll (double & x_ll, double & y_ll) const;

      double x_ll () const;
      double y_ll () const;

      Box cell_box  (int row, int col) const;
      Box row_box   (int row)          const;
      Box col_box   (int col)          const;
      Box table_box ()                 const;

         //
         //  do stuff
         //

      void cell_uv_to_cell_xy(int r, int c, double u, double v, double & x_cell, double & y_cell) const;
      void cell_uv_to_page_xy(int r, int c, double u, double v, double & x_page, double & y_page) const;

      void fill_cell     (int row, int col, const Color &);
      void fill_row      (int row,          const Color &);
      void fill_col      (int col,          const Color &);
      void fill_table    (const Color &);

      void outline_cell  (int row, int col, double linewidth, const Color &);
      void outline_row   (int row,          double linewidth, const Color &);
      void outline_col   (int col,          double linewidth, const Color &);
      void outline_table (double linewidth, const Color &);

      void outline_inside_cell  (int row, int col, double linewidth, const Color &);
      void outline_inside_row   (int row,          double linewidth, const Color &);
      void outline_inside_col   (int col,          double linewidth, const Color &);
      void outline_inside_table (double linewidth, const Color &);


         //
         //  x, y coords are relative to cell box lower-left corner
         //

      void write_xy_to_cell (int r, int c,
                             double x, double y, double u_item, double v_item, 
                             const char * text);

      void write_xy1_to_cell (int r, int c,
                              double x, double y, double u_item, double v_item, 
                              const char * text);

      void write_uv_to_cell (int r, int c,
                             double u_cell, double v_cell, double u_item, double v_item, 
                             const char * text);

      void draw_skeleton (double linewidth);
      void draw_skeleton (double linewidth, const Color &);

};


////////////////////////////////////////////////////////////////////////


inline int TableHelper::nrows() const { return ( Nrows ); }
inline int TableHelper::ncols() const { return ( Ncols ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __PS_TABLE_HELPER_H__  */


////////////////////////////////////////////////////////////////////////


