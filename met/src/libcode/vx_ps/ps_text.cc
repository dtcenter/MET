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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_log.h"
#include "ps_text.h"


////////////////////////////////////////////////////////////////////////


inline double dmax(double a, double b)  { return ( (a > b) ? a : b ); }
inline double dmin(double a, double b)  { return ( (a < b) ? a : b ); }


////////////////////////////////////////////////////////////////////////


static void base_8_string(int, char *);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class VxpsTextNode
   //


////////////////////////////////////////////////////////////////////////


VxpsTextNode::VxpsTextNode()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


VxpsTextNode::~VxpsTextNode()

{

clear();

}


////////////////////////////////////////////////////////////////////////


VxpsTextNode::VxpsTextNode(const VxpsTextNode & n)

{

init_from_scratch();

assign(n);

}


////////////////////////////////////////////////////////////////////////


VxpsTextNode & VxpsTextNode::operator=(const VxpsTextNode & n)

{

if ( this == &n )  return ( * this );

assign(n);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::init_from_scratch()

{

Text = (char *) 0;

next = (VxpsTextNode *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::clear()

{

   //
   //  clear out this node
   //

if ( Text )  { delete [] Text;  Text = (char *) 0; }

Nchars = Nalloc = 0;

FontNumber = -1;
FontSize   = 0.0;

Dx = 0.0;

Left = Right = Top = Bottom = 0.0;

Width = 0.0;

   //
   //  send "clear" message down the chain
   //

if ( next )  { delete next;  next = (VxpsTextNode *) 0; }

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::assign(const VxpsTextNode & n)

{

clear();

FontNumber = n.FontNumber;
FontSize   = n.FontSize;

Dx = n.Dx;

Left   = n.Left;
Right  = n.Right;
Bottom = n.Bottom;
Top    = n.Top;

Width  = n.Width;

if ( n.Text )  set_text(n.Text);

if ( n.next )  {

   next = new VxpsTextNode;

   *next = *(n.next);

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::dump(ostream & out, int indent_depth) const

{

int j, k;
Indent prefix(indent_depth);
char junk[128];


out << prefix << "FontNumber = " << FontNumber << "\n";
out << prefix << "FontSize   = " << FontSize   << "\n";
out << prefix << "Width      = " << Width      << "\n";
out << prefix << "Left       = " << Left       << "\n";
out << prefix << "Right      = " << Right      << "\n";
out << prefix << "Top        = " << Top        << "\n";
out << prefix << "Bottom     = " << Bottom     << "\n";
out << prefix << "Dx         = " << Dx         << "\n";
out << prefix << "Nchars     = " << Nchars     << "\n";
out << prefix << "Nalloc     = " << Nalloc     << "\n";


if ( Text )  {

   out << prefix << "Text       = \"";

   for (j=0; j<Nchars; ++j)  {

      if ( (Text[j] >= 32) && (Text[j] <= 126) )  {

         out.put(Text[j]);

      } else {

         k = (int) (Text[j]);

         if ( k < 0 )  k += 256;

         base_8_string(k, junk);

         out << "\\" << junk;

      }

   }

   out << "\"\n";

} else {

   out << prefix << "Text       = (nul)\n";

}



if ( next )  {

   out << prefix << "next       = ...\n";

   next->dump(out, indent_depth + 1);

} else {

   out << prefix << "next       = (nul)\n";

}


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::extend(int n)

{

if ( n < Nalloc )  return;

int k;
char * u = (char *) 0;

   //
   //  round n up to the next multiple of vx_pstextnode_alloc_inc
   //

k = n/vx_pstextnode_alloc_inc;

if ( n%vx_pstextnode_alloc_inc )  ++k;

n = k*vx_pstextnode_alloc_inc;

   //
   //  allocate and zero out
   //

u = new char [n];

if ( !u )  {

   mlog << Error << "\nVxpstextNode::extend(int) -> memory allocation error!\n\n";

   exit ( 1 );

}

memset(u, 0, n);

Nalloc = n;

   //
   //  copy the old values, if any
   //

if ( Text )   strncpy(u, Text, Nchars);

   //
   //  toss old, grab new
   //

if ( Text )  { delete [] Text;  Text = (char *) 0; }

Text = u;

u = (char *) 0;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::set_text(const char * s)

{

if ( Text )  { delete [] Text;  Text = (char *) 0; }

Nchars = strlen(s);

extend(Nchars + 2);

memset(Text, 0, Nalloc);

strcpy(Text, s);

return;

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::add_char(const AfmCharMetrics & cm)

{

int k = cm.ascii_code;


if ( (k < 0) || (k > 255) )  {

   mlog << Error << "\nVxpsTextNode::add_char() -> bad byte value ... " << k << "\n\n";

   exit ( 1 );

}

extend(Nchars + 2);

Text[Nchars++] = (char) k;

   //
   //  redo bbox  ("Left" is unchanged, unless this is the first character)
   //

Bottom = dmin(Bottom, cm.bbox.B);

Top    = dmax(Top, cm.bbox.T);

Right  = Width + cm.bbox.R;

Width += cm.width;


   //
   //  "Left" is unchanged, unless this is the first character
   //

if ( Nchars == 1 )  Left = cm.bbox.L;


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


double VxpsTextNode::total_width() const

{

double w = 0.0;
const VxpsTextNode * n = this;

while ( n )  {

   w += n->Width;

   w += n->Dx;

   n = n->next;

}

return ( w );

}


////////////////////////////////////////////////////////////////////////


double VxpsTextNode::total_left() const

{

return ( Left );

}


////////////////////////////////////////////////////////////////////////


double VxpsTextNode::total_right() const

{

double r;
const VxpsTextNode * n = this;

r = 0.0;

while ( n->next )  {

   r += n->Width;

   r += n->Dx;

   n = n->next;

}

r += n->Right;


return ( r );

}


////////////////////////////////////////////////////////////////////////


double VxpsTextNode::total_top() const

{

double t = 0.0;
const VxpsTextNode * n = this;


while ( n )  {

   t = dmax(t, n->Top);

   n = n->next;

}

return ( t );

}


////////////////////////////////////////////////////////////////////////


double VxpsTextNode::total_bottom() const

{

double b = 0.0;
const VxpsTextNode * n = this;


while ( n )  {

   b = dmin(b, n->Bottom);

   n = n->next;

}

return ( b );

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::set_font_number(int n)

{

FontNumber = n;

return;

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::set_font_size(double s)


{

FontSize = s;

return;

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::add_link()

{

if ( next )  {

   mlog << Error << "\nVxpsTextNode::add_link() -> link already present!\n\n";

   exit ( 1 );

}

next = new VxpsTextNode;

next->FontNumber = FontNumber;
next->FontSize   = FontSize;


return;

}


////////////////////////////////////////////////////////////////////////


void VxpsTextNode::set_dx(double delta_x)

{

Dx = delta_x;


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void base_8_string(int k, char * out)

{

if ( (k < 0) || (k > 255) )  {

   mlog << Error << "\nbase_8_string() -> bad value ... " << k << "\n\n";

   exit ( 1 );

}

int j;
char junk[16];
char * c = (char *) 0;

c = junk + (sizeof(junk) - 1);

*c-- = (char) 0;

for (j=0; j<3; ++j)  {

   *c-- = '0' + (k%8);

   k /= 8;

}

++c;


   //
   //  done
   //

strcpy(out, c);

return;

}


////////////////////////////////////////////////////////////////////////






