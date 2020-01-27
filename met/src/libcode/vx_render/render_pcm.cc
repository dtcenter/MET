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


static const int table_size = 1537;


////////////////////////////////////////////////////////////////////////


static void set_up_colortable(PSFilter & out, const Pcm & pcm, const RenderInfo & info);


////////////////////////////////////////////////////////////////////////


void render(PSfile & plot, const Pcm & pcm, const RenderInfo & info)

{

plot.gsave();

int j, r, c;
int nx, ny;
double w, h;
unsigned char u;
Color color;
PSFilter *out = (PSFilter *) 0;
PSFilter **v = &out;


nx = pcm.nx();
ny = pcm.ny();

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
         mlog << Error << "\nrender_color_24() -> bad filter: \"" << (info.filter(j)) << "\"\n\n";
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


set_up_colortable(plot.file(),  pcm, info);


w = nx*(info.x_mag());
h = ny*(info.y_mag());

// if ( !(info.encoding) )  plot.file << "/pix " << (image.width) << " string def\n\n";

plot.file() << "\n"
            << (info.x_ll()) << ' ' << (info.y_ll()) << " translate\n"
            << w << ' ' << h << " scale\n\n";

plot.file() << "<<\n"
            << "  /ImageType 1\n"
            << "  /Width " << nx << "\n"
            << "  /Height " << ny << "\n"
            << "  /BitsPerComponent 8\n"
            // << "  /Decode [ 0 " << (pcm.n_colors() - 1) << " ]\n"
            << "  /Decode [ 0 255 ]\n"
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

if ( out == NULL) { return; }

for (r=0; r<ny; ++r)  {

   for (c=0; c<nx; ++c)  {

      u =  pcm.data_getrc(r, c);

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


void set_up_colortable(PSFilter & out, const Pcm & pcm, const RenderInfo & info)

{

int j, k;
Color color;
char table[table_size];
unsigned char u;


memset(table, 0, table_size);


k = 0;

for (j=0; j<(pcm.n_colors()); ++j)  {

   color = pcm.colormap(j);

   if ( info.is_bw() )  color.to_gray();

   u = color.red();

   table[k++] = hex_string[u >> 4];
   table[k++] = hex_string[u & 15];

   u = color.green();

   table[k++] = hex_string[u >> 4];
   table[k++] = hex_string[u & 15];

   u = color.blue();

   table[k++] = hex_string[u >> 4];
   table[k++] = hex_string[u & 15];

}   //  for j



out << "[ /Indexed /DeviceRGB " << (pcm.n_colors() - 1) << "\n"
    << " <" << table << ">\n"
    << "] setcolorspace\n\n";

return;

}


////////////////////////////////////////////////////////////////////////






