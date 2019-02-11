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

#include "pgm.h"
#include "pxm_utils.h"

#include "vx_log.h"
#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


static  int   row_all_same     (const Pgm &, int row);

static  int   col_all_same     (const Pgm &, int col);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Pgm
   //


////////////////////////////////////////////////////////////////////////


Pgm::Pgm()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Pgm::Pgm(const char * filename)

{

init_from_scratch();

if ( !read(filename) )  {

   mlog << Error << "\nPgm::Pgm(const char *filename) -> failed to read file \"" << filename << "\"\n\n";

   exit ( 1 );

}

}


////////////////////////////////////////////////////////////////////////


Pgm::~Pgm()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Pgm::Pgm(const Pgm & p)

{

init_from_scratch();

assign(p);

}


////////////////////////////////////////////////////////////////////////


Pgm & Pgm::operator=(const Pgm & p)

{

if ( this == &p )  return ( * this );

assign(p);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Pgm::clear()

{

clear_common();

return;

}


////////////////////////////////////////////////////////////////////////


void Pgm::init_from_scratch()

{


// PxmBase::init_from_scratch();


return;

}


////////////////////////////////////////////////////////////////////////


void Pgm::assign(const Pgm & p)

{

copy_common(p);

return;

}


////////////////////////////////////////////////////////////////////////


Color Pgm::getrc(int r, int c) const

{

int n;
unsigned char u;
Color color;

n = rc_to_n(r, c);

u = data[n];

color.set_rgb(u, u, u);

return ( color );

}


////////////////////////////////////////////////////////////////////////


Color Pgm::getxy(int x, int y) const

{

int r, c;

c = x;

r = Nrows - 1 - y;

return ( getrc(r, c) );

}


////////////////////////////////////////////////////////////////////////


void Pgm::putrc(const Color & color, int r, int c)

{

int n;

n = rc_to_n(r, c);

data[n] = color.red();

return;

}


////////////////////////////////////////////////////////////////////////


void Pgm::putxy(const Color & color, int x, int y)

{

int r, c;

c = x;

r = Nrows - 1 - y;

putrc(color, r, c);

return;

}


////////////////////////////////////////////////////////////////////////


int Pgm::read(const char * filename)

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

   mlog << Warning << "\nPgm::read() -> unable to read image file \"" << filename << "\"\n\n";

   return ( 0 );

}

   //
   //  copy filename
   //

Name = new char [1 + strlen(filename)];

if ( !Name )  {

   mlog << Warning << "\nPgm::read() -> can't allocate memory for file name\n\n";

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

   mlog << Warning << "\nPgm::read() -> unable to read magic cookie in image file \"" << filename << "\"\n\n";

   clear();

   return ( 0 );

}

if ( (c1 != 'P') || (c2 != '5') )  {

   mlog << Warning << "\nPgm::read() -> bad magic number in image file \"" << filename << "\"\n\n";

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

if ( maxval != 255 )  {

   mlog << Warning << "\nPgm::read() -> bad maxval: \"" << maxval << "\"\n\n";

   clear();

   return ( 0 );

}

   //
   //  Get data
   //

n = Nrows*Ncols;

if ( !(data = new unsigned char [n]) )  {

   mlog << Warning << "\nPgm::read() -> memory allocation error\n\n";

   clear();

   return ( 0 );

}

if ( !in.read((char *) data, n) )  {

   mlog << Warning << "\nPgm::read() -> trouble reading image data\n\n";

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


int Pgm::write(const char * filename) const

{

int j, n;
ofstream out;


   //
   //  open output file
   //

out.open(filename);

if ( !out )  {

   mlog << Warning << "\nPgm::write() -> unable to open output file \"" << filename << "\"\n\n";

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

out << Ncols << " " << Nrows << "\n255\n";

if ( !out )  {

   mlog << Warning << "\nPgm::write() -> trouble writing header\n\n";

   return ( 0 );

}

   //
   //  data
   //

n = Nrows*Ncols;

for (j=0; j<n; ++j)  out.put(data[j]);

if ( !out )  {

   mlog << Warning << "\nPgm::write() -> trouble writing data\n\n";

   return ( 0 );

}

   //
   //  done
   //

out.close();

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void Pgm::all_black()

{

if ( !data )  return;

(void) memset(data, 0, Nrows*Ncols);

return;

}


////////////////////////////////////////////////////////////////////////


void Pgm::all_white()

{

if ( !data )  return;

(void) memset(data, 255, Nrows*Ncols);

return;

}


////////////////////////////////////////////////////////////////////////


void Pgm::set_size_rc(int NR, int NC)

{

clear();

data = new unsigned char [NR*NC];

if ( !data )  {

   mlog << Error << "\nPgm::set_size(int, int) -> memory allocation error\n\n";

   exit ( 1 );

}

Nrows = NR;
Ncols = NC;

Nalloc = Nrows*Ncols;

all_white();

return;

}


////////////////////////////////////////////////////////////////////////


void Pgm::set_size_xy(int NX, int NY)

{

int NR, NC;

NC = NX;

NR = NY;

set_size_rc(NR, NC);

return;

}


////////////////////////////////////////////////////////////////////////


void Pgm::reverse_video()

{

if ( !data )  {

   mlog << Error << "\nvoid Pgm::reverse_video() -> bad image\n\n";

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


void Pgm::rotate(int a)   // a = 0, 1, 2, 3 multiples of 90 degrees

{

a -= 4*(a/4);

if ( a < 0 )  a += 4;

if ( a == 0 )  return;


int row, col;
int Nrows_new, Ncols_new, bytes;
int row_new = 0, col_new = 0;
int n_new, n_old;
unsigned char *u = (unsigned char *) 0;


if ( a == 2 )  {   //  a == 0 has already been checked for

   Ncols_new = Ncols;
   Nrows_new = Nrows;

} else {

   Ncols_new = Nrows;
   Nrows_new = Ncols;

}

bytes = (Nrows)*(Ncols);

if ( !(u = new unsigned char [bytes]) )  {

   mlog << Error << "\nPgm::rotate(int) -> memory allocation error\n\n";

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


void Pgm::autocrop()

{

int pr, pc;
int row_top, row_bot;
int col_left, col_right;
Color color;
Pgm p;



   //
   //  top
   //

row_top = 0;

while ( row_all_same(*this, row_top) && (row_top < Nrows) )  ++row_top;

   //
   //  bottom
   //

row_bot = Nrows - 1;

while ( row_all_same(*this, row_bot) && (row_bot >= 0) )  --row_bot;

   //
   //  left
   //

col_left = 0;

while ( col_all_same(*this, col_left) && (col_left < Ncols) )  ++col_left;

   //
   //  right
   //

col_right = Ncols - 1;

while ( col_all_same(*this, col_right) && (col_right >= 0) )  --col_right;

   //
   //  range check
   //

if (    ( row_top   >= Nrows    )
     || ( row_bot   <  0         )
     || ( col_left  >= Ncols    )
     || ( col_right <  0         )
     || ( col_left  >= col_right )
     || ( row_top   >= row_bot   ) )  {

   mlog << Error << "\nvoid Pgm::autocrop() -> bad image\n\n";

   exit ( 1 );

}

   //
   //  fill in the new image
   //

p.set_size_rc(row_bot - row_top + 1, col_right - col_left + 1);

for (pr=0; pr<(p.nrows()); ++pr)  {

   for (pc=0; pc<(p.ncols()); ++pc)  {

      color = getrc(row_top + pr, col_left + pc);

      p.putrc(color, pr, pc);

   }

}

   //
   //  copy the new image into this image
   //

*this = p;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Pgm::gamma(double g)

{

if ( g <= 0.0 )  {

   mlog << Error << "\nvoid Pgm::gamma(double) -> bad value for gamma ... \"" << g << "\"\n\n";

   exit ( 1 );

}

if ( fabs(g - 1.0) < 1.0e-3 )  return;

int j, k;
int nxy;
unsigned char fudge[256];
double t;
double exponent;


exponent = 1.0/g;


for (j=0; j<256; ++j)  {

   t = ((double) j)/255.0;

   t = pow(t, exponent);

   k = nint(255.0*t);

   fudge[j] = (unsigned char) k;

}

nxy = Nrows*Ncols;

for (j=0; j<nxy; ++j)  {

   data[j] = fudge[data[j]];

}


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Pgm::dump(ostream & out, int depth) const

{

Indent prefix (depth);


out << prefix << "Pgm dump ... \n";
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


int row_all_same(const Pgm & pgm, int row)

{

Color test, color;
int col, nx;

test = pgm.getrc(row, 0);

nx = pgm.nx();

for (col=1; col<nx; ++col)  {

   color = pgm.getrc(row, col);

   if ( color != test )  return ( 0 );

}


return ( 1 );

}


////////////////////////////////////////////////////////////////////////


int col_all_same(const Pgm & pgm, int col)

{

Color test, color;
int row, ny;

test = pgm.getrc(0, col);

ny = pgm.ny();

for (row=1; row<ny; ++row)  {

   color = pgm.getrc(row, col);

   if ( color != test )  return ( 0 );

}


return ( 1 );

}


////////////////////////////////////////////////////////////////////////







