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

#include "indent.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class Indent
   //


////////////////////////////////////////////////////////////////////////


Indent::Indent()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


Indent::~Indent()

{

}


////////////////////////////////////////////////////////////////////////


Indent::Indent(const Indent & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


Indent & Indent::operator=(const Indent & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}

////////////////////////////////////////////////////////////////////////


Indent::Indent(int Depth)

{

init_from_scratch();

depth = Depth;

if ( depth < 0 )  depth = 0;

return;

}


////////////////////////////////////////////////////////////////////////


void Indent::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void Indent::assign(const Indent & i)

{

clear();


depth    = i.depth;

delta    = i.delta;

 on_char = i.on_char;
off_char = i.off_char;


return;

}


////////////////////////////////////////////////////////////////////////


void Indent::clear()

{

depth    = 0;

delta    = default_indent_delta;

 on_char =  default_on_char;
off_char = default_off_char;


return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  code for misc functions
   //


////////////////////////////////////////////////////////////////////////


ostream & operator<<(ostream & s, const Indent & i)

{

int j, jmax;


jmax = (i.delta)*(i.depth);


for (j=0; j<jmax; ++j)  {

   if ( (j%(i.delta)) == 0 )  s.put(i.on_char);
   else                       s.put(i.off_char);

}



return ( s );

}


////////////////////////////////////////////////////////////////////////







