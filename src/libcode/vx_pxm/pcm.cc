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
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


#include "pcm.h"
#include "pxm_utils.h"

#include "vx_log.h"
#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Pcm
   //


////////////////////////////////////////////////////////////////////////


Pcm::Pcm()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Pcm::Pcm(const char * filename)

{

init_from_scratch();

if ( !read(filename) )  {

   mlog << Error << "\nPcm::Pcm(const char *filename) -> failed to read file \"" << filename << "\"\n\n";

   exit ( 1 );

}

}


////////////////////////////////////////////////////////////////////////


Pcm::~Pcm()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Pcm::Pcm(const Pcm & p)

{

init_from_scratch();

assign(p);

}


////////////////////////////////////////////////////////////////////////


Pcm & Pcm::operator=(const Pcm & p)

{

if ( this == &p )  return ( * this );

assign(p);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Pcm::clear()

{

clear_common();

if ( Colormap )  { delete [] Colormap;   Colormap = (Color *) 0; }

Ncolors = 0;


return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::init_from_scratch()

{

// PxmBase::init_from_scratch();


Colormap = (Color *) 0;

Name = (char *) 0;

Ncolors = 0;


// clear();


return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::assign(const Pcm & p)

{

int trouble;


clear();

copy_common(p);


int j;
trouble = 0;


if ( p.Colormap )  {

   Colormap = new Color [p.Ncolors];

   if ( !Colormap )  trouble = 1;

}

if ( trouble )  {

   clear();

   mlog << Error << "\nPcm::assign() -> memory allocation error\n\n";

   exit ( 1 );

}

if ( p.Colormap )  {

   for (j=0; j<Ncolors; ++j)  Colormap[j] = p.Colormap[j];

}

return;

}


////////////////////////////////////////////////////////////////////////


unsigned char Pcm::data_getxy(int x, int y) const

{

int r, c;

c = x;

r = Nrows - 1 - y;

return ( data_getrc(r, c) );

}


////////////////////////////////////////////////////////////////////////


unsigned char Pcm::data_getrc(int r, int c) const

{

int n;

n = rc_to_n(r, c);

return ( data[n] );

}


////////////////////////////////////////////////////////////////////////


Color Pcm::getrc(int r, int c) const

{

int n, k;

n = rc_to_n(r, c);

k = (int) (data[n]);

return ( Colormap[k] );

}


////////////////////////////////////////////////////////////////////////


Color Pcm::getxy(int x, int y) const

{

int r, c;

c = x;

r = Nrows - 1 - y;

return ( getrc(r, c) );

}


////////////////////////////////////////////////////////////////////////


void Pcm::putrc(const Color & color, int r, int c)

{

int n, k;

k = colormap_index(color);

if ( k < 0 )  {

   mlog << Error << "\nPcm::putrc() -> requested color not in colormap!\n\n";

   exit ( 1 );

}

n = rc_to_n(r, c);

data[n] = (unsigned char) k;

return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::putxy(const Color & color, int x, int y)

{

int r, c;

c = x;

r = Nrows - 1 - y;

putrc(color, r, c);

return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::data_putrc(unsigned char u, int r, int c)

{

int n;

n = rc_to_n(r, c);

data[n] = u;

return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::data_putxy(unsigned char u, int x, int y)

{

int r, c;

c = x;

r = Nrows - 1 - y;

data_putrc(u, r, c);

return;

}


////////////////////////////////////////////////////////////////////////


int Pcm::read(const char * filename)

{

int j, n, maxval;
char c1, c2;
char junk[max_comment_length + 10];
ifstream in;
unsigned char r, g, b;


   //
   //  clear out old image, if any
   //

clear();

   //
   //  open input file
   //

in.open(filename);

if ( !in )  {

   mlog << Warning << "\nPcm::read() -> unable to read image file \"" << filename << "\"\n\n";

   return ( 0 );

}

   //
   //  copy filename
   //

Name = new char [1 + strlen(filename)];

if ( !Name )  {

   mlog << Warning << "\nPcm::read() -> can't allocate memory for file name\n\n";

   clear();

   return ( 0 );

}

memcpy(Name, filename, 1 + strlen(filename));

   //
   //  read magic cookie
   //

in.get(c1);
in.get(c2);

if ( !in )  {

   mlog << Warning << "\nPcm::read() -> unable to read magic cookie in image file \"" << filename << "\"\n\n";

   clear();

   return ( 0 );

}

if ( (c1 != 'P') || (c2 != '9') )  {

   mlog << Warning << "\nPcm::read() -> bad magic number in image file \"" << filename << "\"\n\n";

   clear();

   return ( 0 );

}

   //
   //  read comments, if any
   //

skip_whitespace(in);

while ( 1 )  {

   j = in.peek();

   if ( j != '#' )  break;

   get_comment(in, junk);

   add_comment(junk);

}

   //
   //  get width, height, maxval
   //

Ncols = parse_number(in);
Nrows = parse_number(in);
maxval = parse_number(in);

Ncolors = maxval + 1;

/*
if ( maxval != 255 )  {

   mlog << Warning << "\nPcm::read() -> bad maxval: \"" << maxval << "\"\n\n";

   clear();

   return ( 0 );

}
*/

   //
   //  get colormap
   //

Colormap = new Color [Ncolors];

if ( !Colormap )  {

   mlog << Warning << "\nPcm::read() -> can't allocate memory for colormap!\n\n";

   clear();

   return ( 0 );

}

for (j=0; j<Ncolors; ++j)  {

   // in.get(r);
   // in.get(g);
   // in.get(b);

   in.get(c1);   r = (unsigned char) c1;
   in.get(c1);   g = (unsigned char) c1;
   in.get(c1);   b = (unsigned char) c1;

   if ( !in )  {

      mlog << Warning << "\nPcm::read() -> trouble reading colormap data\n\n";

      clear();

      return ( 0 );

   }

   Colormap[j].set_rgb(r, g, b);

}

   //
   //  Get data
   //

n = Nrows*Ncols;

if ( !(data = new unsigned char [n]) )  {

   mlog << Warning << "\nPcm::read() -> memory allocation error\n\n";

   clear();

   return ( 0 );

}

if ( !in.read((char *) data, n) )  {

   mlog << Warning << "\nPcm::read() -> trouble reading image data\n\n";

   clear();

   return ( 0 );

}

in.close();

   //
   //  Done
   //

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int Pcm::write(const char * filename) const

{

int j, n;
ofstream out;


   //
   //  open output file
   //

out.open(filename);

if ( !out )  {

   mlog << Warning << "\nPcm::write() -> unable to open output file \"" << filename << "\"\n\n";

   return ( 0 );

}

   //
   //  magic cookie
   //

out << "P5\n";

   //
   //  comments, if any
   //

for (j=0; j<Ncomments; ++j)  {

   out << "# " << Comment[j] << "\n";

}

   //
   //  width, height, maxval
   //

out << Ncols << " " << Nrows << "\n"
    << Ncolors << "\n";

if ( !out )  {

   mlog << Warning << "\nPcm::write() -> trouble writing header\n\n";

   return ( 0 );

}

   //
   //  colormap
   //

for (j=0; j<Ncolors; ++j)  {

   out.put(Colormap[j].red());
   out.put(Colormap[j].green());
   out.put(Colormap[j].blue());

}

   //
   //  data
   //

n = Nrows*Ncols;

for (j=0; j<n; ++j)  out.put(data[j]);

if ( !out )  {

   mlog << Warning << "\nPcm::write() -> trouble writing data\n\n";

   return ( 0 );

}

   //
   //  done
   //

out.close();

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void Pcm::set_size_rc(int NR, int NC)

{

clear();

data = new unsigned char [NR*NC];

if ( !data )  {

   mlog << Error << "\nPcm::set_size(int, int) -> memory allocation error\n\n";

   exit ( 1 );

}

Nrows = NR;
Ncols = NC;

Nalloc = Nrows*Ncols;

return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::set_size_xy(int NX, int NY)

{

int NR, NC;

NC = NX;

NR = NY;

set_size_rc(NR, NC);

return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::reverse_video()

{

if ( !data )  {

   mlog << Error << "\nvoid Pcm::reverse_video() -> bad image\n\n";

   exit ( 1 );

}

int j, k, n;

n = Nrows*Ncols;

for (j=0; j<n; ++j)  {

   k = (int) data[j];

   k = 255 - k;

   data[j] = (unsigned char) k;

}

return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::rotate(int a)   // a = 0, 1, 2, 3 multiples of 90 degrees

{

a -= 4*(a/4);

if ( a < 0 )  a += 4;

if ( a == 0 )  return;


int row, col;
int Nrows_new, Ncols_new, bytes;
int row_new = 0, col_new = 0;
int n_new, n_old;
unsigned char *u = (unsigned char *) 0;


if ( a == 2 )  {

   Ncols_new = Ncols;
   Nrows_new = Nrows;

} else {

   Ncols_new = Nrows;
   Nrows_new = Ncols;

}

bytes = (Nrows)*(Ncols);

if ( !(u = new unsigned char [bytes]) )  {

   mlog << Error << "\nPcm::rotate(int) -> memory allocation error\n\n";

   exit ( 1 );

}

for (row=0; row<Nrows; ++row)  {

   for (col=0; col<Ncols; ++col)  {

      switch ( a )  {

         case 1:
            row_new = col;
            col_new = Ncols_new - 1 - row;
            break;

         case 2:
            row_new = Nrows_new - 1 - row;
            col_new = Ncols_new - 1 - col;
            break;

         case 3:
            row_new = Nrows_new - 1 - col;
            col_new = row;
            break;

      }   //  switch

      n_old = row*Ncols + col;

      n_new = row_new*Ncols_new + col_new;

      u[n_new] = data[n_old];

   }   //  for col

}   //  for row


Ncols = Ncols_new;
Nrows = Nrows_new;

delete [] data;  data = u;

return;

}


////////////////////////////////////////////////////////////////////////


int Pcm::colormap_index(const Color &color) const

{

int j;

for (j=0; j<Ncolors; ++j)  {

   if ( Colormap[j] == color )  return ( j );

}


return ( -1 );

}


////////////////////////////////////////////////////////////////////////


Color Pcm::colormap(int n) const

{

if ( (n < 0) || (n >= Ncolors) )  {

   mlog << Error << "\nPcm::colormap(int) -> range check error ... value = " << n << "\n\n";

   exit ( 1 );

}

return ( Colormap[n] );

}


////////////////////////////////////////////////////////////////////////


void Pcm::set_colormap(const Color * c, int n)

{

if ( Colormap )  { delete [] Colormap;  Colormap = (Color *) 0; }

Colormap = new Color [n];

if ( !Colormap )  {

   mlog << Error << "\nPcm::set_colormap() -> memory allocation error\n\n";

   exit ( 1 );

}

Ncolors = n;

int j;

for (j=0; j<Ncolors; ++j)  Colormap[j] = c[j];

return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::autocrop()

{

mlog << Error << "\nvoid Pcm::autocrop() -> not yet implemented\n\n";

exit ( 1 );



return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::gamma(double g)

{

if ( g <= 0.0 )  {

   mlog << Error << "\nvoid Pcm::gamma(double) -> bad value for gamma ... \"" << g << "\"\n\n";

   exit ( 1 );

}

if ( fabs(g - 1.0) < 1.0e-3 )  return;

int j, k;
int R, G, B;
unsigned char fudge[256];
double t;
double exponent;
Color color;


exponent = 1.0/g;


for (j=0; j<256; ++j)  {

   t = ((double) j)/255.0;

   t = pow(t, exponent);

   k = nint(255.0*t);

   fudge[j] = (unsigned char) k;

}


for (j=0; j<Ncolors; ++j)  {

   color = Colormap[j];

   R = color.red();
   G = color.green();
   B = color.blue();

   Colormap[j].set_rgb(fudge[R], fudge[G], fudge[B]);

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::dump(ostream & out, int depth) const

{

int j;
Indent prefix (depth);


out << prefix << "Pcm dump ... \n";
out << prefix << "=============\n";

PxmBase::dump(out, depth);

out << prefix << "Ncolors   = " << Ncolors << "\n";

for (j=0; j<Ncolors; ++j)  {

   out << prefix << "Color[" << j << "] = (";

   Colormap[j].dump(out, depth + 1);

}


   //
   //  done
   //

out.flush();


return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::all_black()

{

mlog << Error << "\n\n  Pcm::all_black() -> does nothing!\n\n";

return;

}


////////////////////////////////////////////////////////////////////////


void Pcm::all_white()

{

mlog << Error << "\n\n  Pcm::all_white() -> does nothing!\n\n";

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////






