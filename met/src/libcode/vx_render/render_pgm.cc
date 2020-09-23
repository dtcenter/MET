// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <string.h>

#include "vx_log.h"
#include "vx_render.h"


////////////////////////////////////////////////////////////////////////


void render(PSfile & plot, const Pgm & pgm, const RenderInfo & info)

{

if ( !(info.is_bw()) )  {

   mlog << Error << "\nrender_bw() -> bad info.bw\n\n";

   exit ( 1 );

}

plot.gsave();

int j, r, c;
int nx, ny;
double w, h;
unsigned char u;
Color color;
PSFilter *out = (PSFilter *) 0;
PSFilter **v = &out;


nx = pgm.nx();
ny = pgm.ny();

   //
   //  set up filters
   //

if ( info.n_filters() == 0 )  {

   mlog << Error << "\nrender_color_24() -> must have at least one filter\n\n";

   exit ( 1 );

}

for (j=0; j<(info.n_filters()); ++j)  {

   switch ( info.filter(j) )  {

      case ASCII85Encode:
         *v = new ASCII85EncodeFilter();
         v = &((*v)->next);
         break;

      case HexEncode:
         *v = new HexEncodeFilter();
         v = &((*v)->next);
         break;

      case RunLengthEncode:
         *v = new RunLengthEncodeFilter();
         v = &((*v)->next);
         break;

      default:
         mlog << Error << "\nrender()(pgm) -> bad filter: \"" << (info.filter(j)) << "\"\n\n";
         exit ( 1 );
         break;

   }   //  switch

}   //  for j

   //
   //  put an output filter on the back end
   //

PSOutputFilter * psout = new PSOutputFilter(plot.psout);

psout->ignore_columns = false;

*v = psout;

v = (PSFilter **) 0;




w = nx*(info.x_mag());
h = ny*(info.y_mag());

plot.file() << "/DeviceGray setcolorspace\n\n";

// if ( !(info.encoding) )  plot.file << "/pix " << (image.width) << " string def\n\n";

plot.file() << (info.x_ll()) << ' ' << (info.y_ll()) << " translate\n"
            << w << ' ' << h << " scale\n\n";

plot.file() << "<<\n"
            << "  /ImageType 1\n"
            << "  /Width " << nx << "\n"
            << "  /Height " << ny << "\n"
            << "  /BitsPerComponent 8\n"
            << "  /Decode [ 0 1 ]\n"
            << "  /ImageMatrix [ " << nx << " 0 0 " << -ny << " 0 " << ny << " ]\n"
            << "  /DataSource currentfile ";

for (j=(info.n_filters() - 1); j>= 0; --j)  {

   switch ( info.filter(j) )  {

      case ASCII85Encode:
         plot.file() << "/ASCII85Decode filter ";
         break;

      case HexEncode:
         plot.file() << "/ASCIIHexDecode filter ";
         break;

      case RunLengthEncode:
         plot.file() << "/RunLengthDecode filter ";
         break;

      default:
         mlog << Error << "\nrender() -> bad filter: \"" << (info.filter(j)) << "\"\n\n";
         exit ( 1 );
         break;

   }   //  swtich

}

plot.file() << "\n"
            <<  ">>\n\nimage\n\n";

if ( out == NULL ) { return; }
 
for (r=0; r<ny; ++r)  {

   for (c=0; c<nx; ++c)  {

      color = pgm.getrc(r, c);

      // u = color_to_gray(color);
      u = color.red();

      // if ( u != 255 )  mlog << Debug(1) << ((int) u) << "\n";

      out->eat(u);

   }   //  for c

}   //  for r

out->eod();

delete out;   out = (PSFilter *) 0;

plot.file() << "\n\n";

   //
   //  done
   //

plot.grestore();

return;

}


////////////////////////////////////////////////////////////////////////





