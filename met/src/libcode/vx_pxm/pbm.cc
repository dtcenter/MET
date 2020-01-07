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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "vx_log.h"
#include "pbm.h"
#include "pxm_utils.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Pbm
   //


////////////////////////////////////////////////////////////////////////


Pbm::Pbm()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Pbm::Pbm(const char * filename)

{

init_from_scratch();

if ( !read(filename) )  {

   mlog << Error << "\nPbm::Pbm(const char *filename) -> failed to read file \"" << filename << "\"\n\n";

   exit ( 1 );

}

}


////////////////////////////////////////////////////////////////////////


Pbm::~Pbm()

{

clear();

}


////////////////////////////////////////////////////////////////////////


Pbm::Pbm(const Pbm & p)

{

init_from_scratch();

assign(p);

}


////////////////////////////////////////////////////////////////////////


Pbm & Pbm::operator=(const Pbm & p)

{

if ( this == &p )  return ( * this );

assign(p);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void Pbm::clear()

{

clear_common();

return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::init_from_scratch()

{

// PxmBase::init_from_scratch();

return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::assign(const Pbm & p)

{

copy_common(p);

return;

}


////////////////////////////////////////////////////////////////////////


Color Pbm::getrc(int r, int c) const

{

   //
   //  Note: in pbms 1 means black, 0 means white.
   //  We want it the other way, though
   //

if ( (r < 0) || (r >= Nrows) || (c < 0) || (c >= Ncols) )  {

   mlog << Error << "\nPbm::getrc() -> range check error!\n\n";

   exit ( 1 );

   // return ( 0 );

}

int j, n;
unsigned char u, mask;
Color color;


n = r*bytes_per_row() + c/8;

mask = 1 << (7 - c%8);

u = data[n];

   //
   //  Note: in pbms 1 means black, 0 means white.
   //  We want it the other way, though
   //

// j = ( (u & mask) ? 1 : 0 );

j = ( (u & mask) ? 0 : 1 );

if ( j )   color.set_gray(255);
else       color.set_gray(0);

return ( color );

}


////////////////////////////////////////////////////////////////////////


Color Pbm::getxy(int x, int y) const

{

int r, c;
Color color;


c = x;

r = Nrows - 1 - y;

color = getrc(r, c);

return ( color );

}


////////////////////////////////////////////////////////////////////////


void Pbm::putrc(const Color & color, int r, int c)

{

int n;
unsigned char mask;
int value;

   //
   //  Note in pbm's 1 means black, 0 means white.
   //  We want it the other way, though
   //

if ( (r < 0) || (r >= Nrows) || (c < 0) || (c >= Ncols) )  {

   mlog << Error << "\nPbm::putrc() -> range check error!\n\n";

   exit ( 1 );

   // return;

}

value = (int) (color.red());

if ( value > 0 )  value = 1;

value = !value;

n = r*bytes_per_row() + c/8;

mask = 1 << (7 - c%8);

if ( value )  {

   data[n] |= mask;

} else {

   mask = ~mask;

   data[n] &= mask;

}

return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::putxy(const Color & color, int x, int y)

{

int r, c;

c = x;

r = Nrows - 1 - y;

putrc(color, r, c);

return;

}


////////////////////////////////////////////////////////////////////////


int Pbm::read(const char *filename)

{

int n, j;
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

   mlog << Warning << "\nPbm::read() -> unable to read image file \"" << filename << "\"\n\n";

   return ( 0 );

}

   //
   //  copy filename
   //

Name = new char [1 + strlen(filename)];

if ( !Name )  {

   mlog << Warning << "\nPbm::read() -> can't allocate memory for file name\n\n";

   clear();

   return ( 0 );

}

strcpy(Name, filename);

   //
   //  read magic cookie
   //

in.get(c1);
in.get(c2);

if ( !in )  {

   mlog << Warning << "\nPbm::read() -> unable to read magic cookie in image file \"" << filename << "\"\n\n";

   clear();

   return ( 0 );

}

if ( (c1 != 'P') || (c2 != '4') )  {

   mlog << Warning << "\nPbm::read() -> bad magic number in image file \"" << filename << "\"\n\n";

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
   //  get width, height
   //

Ncols = parse_number(in);
Nrows = parse_number(in);

   //
   //  Get data
   //

n = total_data_bytes();

if ( !(data = new unsigned char [n]) )  {

   mlog << Warning << "\nPbm::read() -> memory allocation error\n\n";

   clear();

   return ( 0 );

}

if ( !in.read((char *) data, n) )  {

   mlog << Warning << "\nPbm::read() -> trouble reading image data\n\n";

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


int Pbm::write(const char * filename) const

{

int j, n;
ofstream out;


   //
   //  open output file
   //

out.open(filename);

if ( !out )  {

   mlog << Warning << "\nPbm::write() -> unable to open output file \"" << filename << "\"\n\n";

   return ( 0 );

}

   //
   //  magic cookie
   //

out << "P4\n";

   //
   //  comments, if any
   //

for (j=0; j<Ncomments; ++j)  {

   out << "# " << Comment[j] << "\n";

}

   //
   //  width, height
   //

out << Ncols << " " << Nrows << "\n";

if ( !out )  {

   mlog << Warning << "\nPbm::write() -> trouble writing header\n\n";

   return ( 0 );

}

   //
   //  data
   //

n = total_data_bytes();

for (j=0; j<n; ++j)  out.put(data[j]);

if ( !out )  {

   mlog << Warning << "\nPbm::write() -> trouble writing data\n\n";

   return ( 0 );

}

   //
   //  done
   //

out.close();

return ( 1 );

}


////////////////////////////////////////////////////////////////////////


void Pbm::all_black()

{

   //
   //  Note in pbm's 1 means black, 0 means white.
   //  We want it the other way, though
   //

if ( !data )  return;

(void) memset(data, 255, total_data_bytes());

return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::all_white()

{

   //
   //  Note in pbm's 1 means black, 0 means white.
   //  We want it the other way, though
   //

if ( !data )  return;

(void) memset(data, 0, total_data_bytes());

return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::set_size_rc(int NR, int NC)

{

clear();

Nrows = NR;
Ncols = NC;

data = new unsigned char [total_data_bytes()];

if ( !data )  {

   mlog << Error << "\nPbm::set_size(int, int) -> memory allocation error\n\n";

   exit ( 1 );

}

Nalloc = total_data_bytes();

all_white();

return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::set_size_xy(int NX, int NY)

{

int NR, NC;


NC = NX;

NR = NY;

set_size_rc(NR, NC);


return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::reverse_video()

{

if ( !data )  {

   mlog << Error << "\nvoid Pbm::reverse_video() -> bad image\n\n";

   exit ( 1 );

}

int j, n;
unsigned char u;

n = total_data_bytes();

for (j=0; j<n; ++j)  {

   u = data[j];

   data[j] = ~u;

}

return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::rotate(int)

{


mlog << Error << "\nPbm::rotate(int) -> not yet implemented ... sorry\n\n";

exit ( 1 );


return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::autocrop()

{


mlog << Error << "\nPbm::autocrop() -> not yet implemented ... sorry\n\n";

exit ( 1 );


return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::gamma(double)

{

   //
   //  gamma-correcting a bitmap does nothing
   //

return;

}


////////////////////////////////////////////////////////////////////////


void Pbm::dump(ostream & out, int depth) const

{

Indent prefix (depth);


out << prefix << "Pbm dump ... \n";
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







