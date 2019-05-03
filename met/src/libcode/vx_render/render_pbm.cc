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
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <string.h>

#include "vx_log.h"
#include "vx_render.h"


////////////////////////////////////////////////////////////////////////


void render(PSfile & plot, const Pbm & pbm, const RenderInfo & info)

{

if ( !(info.is_bw()) )  {

   mlog << Error << "\nrender_bw() -> bad info.bw\n\n";

   exit ( 1 );

}

plot.gsave();

int j, r, c;
int nx, ny;
int pad;
double w, h;
unsigned char u;
Color color;
PSFilter *out = (PSFilter *) 0;
PSFilter **v = &out;


nx = pbm.nx();
ny = pbm.ny();

   //
   //  set up filters
   //

if ( info.n_filters() == 0 )  {

   mlog << Error << "\nrender_color_24() -> must have at least one filter\n\n";

   exit ( 1 );

}

   //
   //  put bitfilter on front
   //

BitFilter *b = new BitFilter;

b->bits_per_component = 1;

*v = b;
v = &((*v)->next);

   //
   //  loop through filters
   //

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
         mlog << Error << "\nrender()(pbm) -> bad filter: \"" << (info.filter(j)) << "\"\n\n";
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
            << "  /BitsPerComponent 1\n"
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

pad = 8 - nx%8;

if ( pad == 8 )  pad = 0;

if ( out == NULL) { return; }

for (r=0; r<ny; ++r)  {

   for (c=0; c<nx; ++c)  {

      color = pbm.getrc(r, c);

      u = color.red();

      if ( u > 0 )  u = 1;

      out->eat(u);

   }   //  for c

   for (j=0; j<pad; ++j)  out->eat(0);

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







