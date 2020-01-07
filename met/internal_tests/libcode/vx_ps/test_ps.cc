// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

   //
   //  use "/usr/bin/gv -media=automatic" to view the output files
   //

static const int font_number = 12;

static const double font_size = 50.0;

static const double h_margin = 72.0;   //  border margins
static const double v_margin = 72.0;   //  border margins


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"
#include "vx_ps.h"
#include "documentmedia_to_string.h"
#include "documentorientation_to_string.h"


////////////////////////////////////////////////////////////////////////


static void test_plot(const char *, DocumentMedia, DocumentOrientation);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

test_plot("1.ps", MediaLetter, OrientationPortrait);
test_plot("2.ps", MediaLetter, OrientationLandscape);

test_plot("3.ps", MediaA4, OrientationPortrait);
test_plot("4.ps", MediaA4, OrientationLandscape);


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void test_plot(const char * filename, DocumentMedia DM, DocumentOrientation DO)

{

PSfile plot;
double x_ll, y_ll, w, h;
ConcatString s;

   //
   //  open the output file
   //

plot.open(filename, DM, DO);

plot.pagenumber(1);

   //
   //  Hello!
   //

plot.choose_font(font_number, font_size);

plot.write_centered_text(2, 1, 0.5*(plot.page_width()), 0.5*(plot.page_height()), 0.5, 0.5, "Hello!");

   //
   //  show params
   //

plot.choose_font(7, 10.0);

s = documentmedia_to_string(plot.media());

plot.write_centered_text(1, 1, 0.5*(plot.page_width()), v_margin + 40.0, 0.5, 0.0, s.c_str());

s = documentorientation_to_string(plot.orientation());

plot.write_centered_text(1, 1, 0.5*(plot.page_width()), v_margin + 20.0, 0.5, 0.0, s.c_str());

   //
   //  border
   //

x_ll = h_margin;
y_ll = v_margin;

w = plot.page_width()  - 2.0*h_margin;
h = plot.page_height() - 2.0*v_margin;

plot.setlinewidth(2.0);

plot.newpath();

plot.moveto(x_ll,     y_ll);
plot.lineto(x_ll + w, y_ll);
plot.lineto(x_ll + w, y_ll + h);
plot.lineto(x_ll,     y_ll + h);

plot.closepath();
plot.stroke();

   //
   //  done
   //

plot.showpage();

return;

}


////////////////////////////////////////////////////////////////////////


