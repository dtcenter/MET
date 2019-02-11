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
#include <errno.h>

#include "ppm.h"
#include "pxm_utils.h"

#include "vx_log.h"
#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Ppm
   //


////////////////////////////////////////////////////////////////////////


Ppm::Ppm()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Ppm::Ppm(const char * filename)

{

init_from_scratch();

if ( !read(filename) )  {

   mlog << Error << "\nPpm::Ppm(const char *) -> failed to read file \"" << filename << "\"\n\n";

   exit ( 1 );

}

}


////////////////////////////////////////////////////////////////////////


Ppm::~Ppm()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Ppm::Ppm(const Ppm &p)

{

init_from_scratch();

assign(p);

}


////////////////////////////////////////////////////////////////////////


Ppm & Ppm::operator=(const Ppm & p)

{

if ( this == &p )  return ( * this );

assign(p);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Ppm::clear()

{

clear_common();

return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::init_from_scratch()

{


// PxmBase::init_from_scratch();


return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::assign(const Ppm & p)

{

copy_common(p);

return;

}


////////////////////////////////////////////////////////////////////////


Color Ppm::getrc(int r, int c) const

{

int n;
unsigned char * u = (unsigned char *) 0;
Color color;


n = rc_to_n(r, c);

u = data + 3*n;

color.set_rgb(u[0], u[1], u[2]);

return ( color );

}


////////////////////////////////////////////////////////////////////////


Color Ppm::getxy(int x, int y) const

{

int r, c;

c = x;

r = Nrows - 1 - y;

return ( getrc(r, c) );

}


////////////////////////////////////////////////////////////////////////


void Ppm::putrc(const Color & color, int r, int c)

{

int n;
unsigned char * u = (unsigned char *) 0;


n = rc_to_n(r, c);

u = data + 3*n;

u[0] = color.red();
u[1] = color.green();
u[2] = color.blue();

return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::putxy(const Color & color, int x, int y)

{

int r, c;

c = x;

r = Nrows - 1 - y;

putrc(color, r, c);

return;

}


////////////////////////////////////////////////////////////////////////


int Ppm::read(const char * filename)

{

int j, n, maxval;
char c1, c2;
char junk[max_comment_length + 10];
ifstream in;


   //
   //  clear out old image, if any
   //

clear();

   //
   //  open input file
   //

in.open(filename);

if ( !in )  {

   mlog << Error << "\nPpm::read() -> unable to read image file \"" << filename << "\"\n\n";

   return ( 0 );

}

   //
   //  copy filename
   //

Name = new char [1 + strlen(filename)];

if ( !Name )  {

   mlog << Error << "\nPpm::read() -> can't allocate memory for file name\n\n";

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

   mlog << Error << "\nPpm::read() -> unable to read magic cookie in image file \"" << filename << "\"\n\n";

   clear();

   return ( 0 );

}

if ( (c1 != 'P') || (c2 != '6') )  {

   mlog << Error << "\nPpm::read() -> bad magic number in image file \"" << filename << "\"\n\n";

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

Ncols  = parse_number(in);
Nrows  = parse_number(in);
maxval = parse_number(in);

if ( maxval != 255 )  {

   mlog << Error << "\nPpm::read() -> bad maxval: \"" << maxval << "\"\n\n";

   clear();

   return ( 0 );

}

   //
   //  Get data
   //

n = 3*Nrows*Ncols;

if ( !(data = new unsigned char [n]) )  {

   mlog << Error << "\nPpm::read() -> memory allocation error\n\n";

   clear();

   return ( 0 );

}

if ( !in.read((char *) data, n) )  {

   mlog << Error << "\nPpm::read() -> trouble reading image data\n\n";

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


int Ppm::write(const char * filename) const

{

int j, n;
ofstream out;


   //
   //  open output file
   //

out.open(filename);

if ( !out )  {

   mlog << Warning << "\nPpm::write() -> unable to open output file \"" << filename << "\"\n\n";

   return ( 0 );

}

   //
   //  magic cookie
   //

out << "P6\n";

   //
   //  comments, if any
   //

for (j=0; j<Ncomments; ++j)  {

   out << "# " << Comment[j] << "\n";

}

   //
   //  width, height, maxval
   //

out << Ncols << " " << Nrows << "\n255\n";

if ( !out )  {

   mlog << Warning << "\nPpm::write() -> trouble writing header ... "
        // << sys_errlist[errno]
        << strerror(errno)
        << "\n\n";

   return ( 0 );

}

   //
   //  data
   //

n = 3*Nrows*Ncols;

out.write((char *) data, n);

if ( !out )  {

   mlog << Warning << "\nPpm::write() -> trouble writing data\n\n";

   return ( 0 );

}

   //
   //  done
   //

out.close();

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void Ppm::all_black()

{

if ( !data )  return;

(void) memset(data, 0, 3*Nrows*Ncols);

return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::all_white()

{

if ( !data )  return;

(void) memset(data, 255, 3*Nrows*Ncols);

return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::set_size_rc(int NR, int NC)

{

clear();

data = new unsigned char [3*NR*NC];

if ( !data )  {

   mlog << Error << "\nPpm::set_size(int, int) -> memory allocation error\n\n";

   exit ( 1 );

}

Nrows = NR;
Ncols = NC;

Nalloc = 3*Nrows*Ncols;

all_white();

return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::set_size_xy(int NX, int NY)

{

int NR, NC;

NC = NX;

NR = NY;

set_size_rc(NR, NC);

return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::reverse_video()

{

if ( !data )  {

   mlog << Error << "\nvoid Ppm::reverse_video() -> bad image\n\n";

   exit ( 1 );

}

int j, k, n;

n = 3*Nrows*Ncols;

for (j=0; j<n; ++j)  {

   k = (int) data[j];

   k = 255 - k;

   data[j] = (unsigned char) k;

}

return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::make_gray()

{

int j, n;
Color c;
unsigned char * u = (unsigned char *) 0;


n = Nrows*Ncols;

u = data;

for (j=0; j<n; ++j)  {

   c.set_rgb(u[0], u[1], u[2]);

   u[0] = u[1] = u[2] = color_to_gray(c);

   u += 3;

}

return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::rotate(int a)   //  a = 0, 1, 2, 3 multiples of 90 degrees

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

bytes = 3*(Nrows)*(Ncols);

if ( !(u = new unsigned char [bytes]) )  {

   mlog << Error << "\nPpm::rotate(int) -> memory allocation error\n\n";

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

      n_old = 3*(row*Ncols + col);

      n_new = 3*(row_new*Ncols_new + col_new);

      u[n_new] = data[n_old];
      u[n_new + 1] = data[n_old + 1];
      u[n_new + 2] = data[n_old + 2];

   }   //  for col

}   //  for row


Ncols = Ncols_new;
Nrows = Nrows_new;

delete [] data;  data = u;

return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::autocrop()

{


mlog << Error << "\nvoid Ppm::autocrop() -> not yet implemented\n\n";

exit ( 1 );


return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::gamma(double g)

{

if ( g <= 0.0 )  {

   mlog << Error << "\nvoid Ppm::gamma(double) -> bad value for gamma ... \"" << g << "\"\n\n";

   exit ( 1 );

}

if ( fabs(g - 1.0) < 1.0e-3 )  return;

int j, k;
int nxy;
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

nxy = 3*Ncols*Nrows;

for (j=0; j<nxy; ++j)  {

   data[j] = fudge[data[j]];

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Ppm::dump(ostream & out, int depth) const

{

Indent prefix (depth);


out << prefix << "Ppm dump ... \n";
out << prefix << "=============\n";
PxmBase::dump(out, depth);


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////




