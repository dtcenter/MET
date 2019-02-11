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
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"
#include "vx_math.h"
#include "vx_color.h"
#include "table_helper.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const double default_col_width  = 50.0;
static const double default_row_height = 50.0;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class TableHelper
   //


////////////////////////////////////////////////////////////////////////


TableHelper::TableHelper()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


TableHelper::~TableHelper()

{

clear();

}


////////////////////////////////////////////////////////////////////////


TableHelper::TableHelper(const TableHelper &)

{

mlog << Error
     << "\n\n  TableHelper::TableHelper(const TableHelper &) -> should never be called!\n\n";

exit ( 1 );

// init_from_scratch();
// 
// assign(t);

}


////////////////////////////////////////////////////////////////////////


TableHelper & TableHelper::operator=(const TableHelper &)

{

mlog << Error
     << "\n\n  TableHelper::operator=(const TableHelper &) -> should never be called!\n\n";

exit ( 1 );

// if ( this == &t )  returnt ( * this );
// 
// assign(t);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void TableHelper::init_from_scratch()

{

ColWidth = (double *) 0;

RowHeight = (double *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::clear()

{

Plot = (PSfile *) 0;   //  not allocated, so don't delete

if ( ColWidth )  { delete [] ColWidth;  ColWidth = (double *) 0; }

if ( RowHeight )  { delete [] RowHeight;  RowHeight = (double *) 0; }

Xpin = Ypin = Upin = Vpin = 0.0;

PinIsSet = false;

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::set(PSfile & iplot, int _nrows, int _ncols)

{

clear();

Plot = &iplot;

Nrows = _nrows;
Ncols = _ncols;

if ( (Nrows <= 0) || (Ncols <= 0) )  {

   mlog << Error
        << "\n\n  TableHelper::set_size_rc() -> bad nrows or ncols\n\n";

   exit ( 1 );


}

int j;


ColWidth  = new double [Ncols];
RowHeight = new double [Nrows];

for (j=0; j<Ncols; ++j)  ColWidth[j]  = default_col_width;
for (j=0; j<Nrows; ++j)  RowHeight[j] = default_row_height;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::range_check_rc(int row, int col) const

{

if ( (row < 0) || (row >= Nrows) || (col < 0) || (col >= Ncols) )  {

   mlog << Error
        << "\n\n  TableHelper::range_check_rc(int row, int col) const -> range check error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::range_check_r(int row) const

{

if ( (row < 0) || (row >= Nrows) )  {

   mlog << Error
        << "\n\n  TableHelper::range_check_r(int row) const -> range check error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::range_check_c(int col) const

{

if ( (col < 0) || (col >= Ncols) )  {

   mlog << Error << "\n\n  TableHelper::range_check_c(int col) const -> range check error\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::check_plot() const

{

if ( !Plot )  {

   mlog << Error << "\n\n  void TableHelper::check_plot() const -> no plot set!\n\n";

   exit ( 1 );

}


return;

}


////////////////////////////////////////////////////////////////////////


double TableHelper::col_width(int c) const

{

range_check_c(c);

return ( ColWidth[c] );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::row_height(int r) const

{

range_check_r(r);

return ( RowHeight[r] );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::width() const

{

int j;
double w = 0.0;

for (j=0; j<Ncols; ++j)  w += ColWidth[j];

return ( w );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::height() const

{

int j;
double h = 0.0;

for (j=0; j<Nrows; ++j)  h += RowHeight[j];

return ( h );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::left() const

{

return ( x_ll() );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::right() const

{

return ( x_ll() + width() );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::top() const

{

return ( y_ll() +height() );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::bottom() const

{

return ( y_ll() );

}


////////////////////////////////////////////////////////////////////////


void TableHelper::cell_ll(int r, int c, double & cx_ll, double & cy_ll) const

{

check_plot();

range_check_rc(r, c);

int j;

cx_ll = x_ll();

cy_ll = y_ll() + height();

for (j=0; j<c; ++j)  cx_ll += ColWidth[j];

for (j=0; j<=r; ++j)  cy_ll -= RowHeight[j];

return;

}


////////////////////////////////////////////////////////////////////////


Box TableHelper::cell_box(int r, int c) const

{

Box b;
double cx_ll, cy_ll;

cell_ll(r, c, cx_ll, cy_ll);

b.set_llwh(cx_ll, cy_ll, col_width(c), row_height(r));

return ( b );

}


////////////////////////////////////////////////////////////////////////


Box TableHelper::col_box(int c) const

{

Box b;
double cx_ll, cy_ll;

cell_ll(Nrows - 1, c, cx_ll, cy_ll);

b.set_llwh(cx_ll, cy_ll, col_width(c), height());

return ( b );

}


////////////////////////////////////////////////////////////////////////


Box TableHelper::row_box(int r) const

{

Box b;
double rx_ll, ry_ll;

cell_ll(r, 0, rx_ll, ry_ll);

b.set_llwh(rx_ll, ry_ll, width(), row_height(r));

return ( b );

}


////////////////////////////////////////////////////////////////////////


Box TableHelper::table_box() const

{

Box b;

b.set_llwh(x_ll(), y_ll(), width(), height());

return ( b );

}


////////////////////////////////////////////////////////////////////////


void TableHelper::fill_cell(int r, int c, const Color & color)

{

check_plot();

const Box b = cell_box(r, c);

Plot->fill_box(b, color);

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::fill_col(int c, const Color & color)

{

check_plot();

const Box b = col_box(c);

Plot->fill_box(b, color);

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::fill_row(int r, const Color & color)

{

check_plot();

const Box b = row_box(r);

Plot->fill_box(b, color);

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::fill_table(const Color & color)

{

check_plot();

const Box b = table_box();

Plot->fill_box(b, color);

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::outline_cell(int row, int col, double linewidth, const Color & color)

{

check_plot();

const Box b = cell_box(row, col);

Plot->outline_box(b, color, linewidth);

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::outline_row(int row, double linewidth, const Color & color)

{

check_plot();

const Box b = row_box(row);

Plot->outline_box(b, color, linewidth);

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::outline_col(int col, double linewidth, const Color & color)

{

check_plot();

const Box b = col_box(col);

Plot->outline_box(b, color, linewidth);

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::outline_table (double linewidth, const Color & color)

{

check_plot();

const Box b = table_box();

Plot->outline_box(b, color, linewidth);

return;

}


////////////////////////////////////////////////////////////////////////



void TableHelper::outline_inside_cell(int row, int col, double linewidth, const Color & color)

{

check_plot();

Box b = cell_box(row, col);

b.shrink(0.5*linewidth);

Plot->gsave();

   Plot->setlinejoin_miter();

   Plot->outline_box(b, color, linewidth);

Plot->grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::outline_inside_row(int row, double linewidth, const Color & color)

{

check_plot();

Box b = row_box(row);

b.shrink(0.5*linewidth);

Plot->gsave();

   Plot->setlinejoin_miter();

   Plot->outline_box(b, color, linewidth);

Plot->grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::outline_inside_col(int col, double linewidth, const Color & color)

{

check_plot();

Box b = col_box(col);

b.shrink(0.5*linewidth);

Plot->gsave();

   Plot->setlinejoin_miter();

   Plot->outline_box(b, color, linewidth);

Plot->grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::outline_inside_table (double linewidth, const Color & color)

{

check_plot();

Box b = table_box();

b.shrink(0.5*linewidth);

Plot->gsave();

   Plot->setlinejoin_miter();

   Plot->outline_box(b, color, linewidth);

Plot->grestore();

return;

}



////////////////////////////////////////////////////////////////////////


void TableHelper::set_cell_wh(double w, double h)

{

if ( PinIsSet )  {

   mlog << Error << "\n\n  TableHelper::set_cell_wh() -> operation not permitted after pin is set!\n\n";

   exit ( 1 );

}

int j;

for (j=0; j<Nrows; ++j)  RowHeight[j] = h;

for (j=0; j<Ncols; ++j)  ColWidth[j] = w;

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::set_col_width(int c, double w)

{

if ( PinIsSet )  {

   mlog << Error << "\n\n  TableHelper::set_col_width() -> operation not permitted after pin is set!\n\n";

   exit ( 1 );

}

range_check_c(c);

if ( w <= 0.0 )  {

   mlog << Error << "\n\n  TableHelper::set_col_width(int c, double w) -> bad width ... " << w << "\n\n";

   exit ( 1 );

}

ColWidth[c] = w;

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::set_row_height(int r, double h)

{

if ( PinIsSet )  {

   mlog << Error << "\n\n  TableHelper::set_row_height() -> operation not permitted after pin is set!\n\n";

   exit ( 1 );

}

range_check_r(r);

if ( h < 0.0 )  {

   mlog << Error << "\n\n  TableHelper::set_row_height(int r, double w) -> bad height ... " << h << "\n\n";

   exit ( 1 );

}

RowHeight[r] = h;

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::draw_skeleton(double linewidth)

{

check_plot();

int j;
double x, y;
const Box t = table_box();

// outline_box(t, linewidth);

Plot->gsave();

   Plot->setlinewidth(linewidth);

   Plot->setlinecap_butt();

   x = t.left() + ColWidth[0];

   for (j=1; j<Ncols; ++j)  {

      Plot->line(x, t.bottom(), x, t.top());

      x += ColWidth[j];

   }

   y = t.top() - RowHeight[0];

   for (j=1; j<Nrows; ++j)  {

      Plot->line(t.left(), y, t.right(), y);

      y -= RowHeight[j];

   }

Plot->grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::draw_skeleton(double linewidth, const Color & color)

{

check_plot();

Plot->gsave();

   Plot->set_color(color);

   draw_skeleton(linewidth);

Plot->grestore();

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::set_pin(double x, double y, double u, double v)

{

Xpin = x;
Ypin = y;

Upin = u;
Vpin = v;

PinIsSet = true;

return;

}


////////////////////////////////////////////////////////////////////////


double TableHelper::x_ll() const

{

return ( Xpin - Upin*width() );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::y_ll() const

{

return ( Ypin - Vpin*height() );

}


////////////////////////////////////////////////////////////////////////


void TableHelper::table_ll(double & tx_ll, double & ty_ll) const

{

tx_ll = x_ll();
ty_ll = y_ll();

return;

}


////////////////////////////////////////////////////////////////////////


double TableHelper::row_top(int r) const

{

range_check_r(r);

int j;
double y = top();

for (j=0; j<r; ++j)  {

   y -= RowHeight[j];

}

   //
   //  done
   //

return ( y );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::row_bottom(int r) const

{

if ( r == (Nrows - 1) )  return ( y_ll() );

return ( row_top(r + 1) );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::col_left(int c) const

{

range_check_c(c);

int j;
double x = x_ll();

for (j=0; j<c; ++j)  {

   x += ColWidth[j];

}

   //
   //  done
   //

return ( x );

}


////////////////////////////////////////////////////////////////////////


double TableHelper::col_right(int c) const

{

if ( c == (Ncols - 1) )  return ( x_ll() + width() );

return ( col_left(c + 1) );

}


////////////////////////////////////////////////////////////////////////


void TableHelper::write_xy_to_cell (int r, int c,
                                    double x, double y, double u, double v,
                                    const char * text)

{

check_plot();

const Box box = cell_box(r, c);

Plot->write_centered_text(2, 1, box.left() + x, box.bottom() + y, u, v, text);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::write_xy1_to_cell (int r, int c,
                                     double x, double y, double u, double v,
                                     const char * text)

{

check_plot();

const Box box = cell_box(r, c);
const char * t = 0;

if ( text )  t = text;

Plot->write_centered_text(1, 1, box.left() + x, box.bottom() + y, u, v, t);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::write_uv_to_cell (int r, int c,
                                    double u_cell, double v_cell, double u_item, double v_item,
                                    const char * text)

{

check_plot();

double x, y;
const Box box = cell_box(r, c);

x = box.left()   + u_cell*(box.width());
y = box.bottom() + v_cell*(box.height());

write_xy_to_cell(r, c, x, y, u_item, v_item, text);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::cell_uv_to_cell_xy(int r, int c, double u, double v, double & x_cell, double & y_cell) const

{

Box b = cell_box(r, c);

x_cell = u*(b.width());
y_cell = v*(b.height());

return;

}


////////////////////////////////////////////////////////////////////////


void TableHelper::cell_uv_to_page_xy(int r, int c, double u, double v, double & x_page, double & y_page) const

{

Box b = cell_box(r, c);

x_page = b.left()   + u*(b.width());
y_page = b.bottom() + v*(b.height());


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////




