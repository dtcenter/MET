

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2010
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

#include "var_info.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class VarInfo
   //


////////////////////////////////////////////////////////////////////////


VarInfo::VarInfo()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


VarInfo::~VarInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


VarInfo::VarInfo(const VarInfo & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


VarInfo & VarInfo::operator=(const VarInfo & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void VarInfo::init_from_scratch()

{

Dims = (NcDim **) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void VarInfo::clear()

{

var = (NcVar *) 0;   //  don't delete

name.clear();

level.clear();

units.clear();

AccumTime = 0;

Ndims = 0;

if ( Dims )  { delete [] Dims;  Dims = (NcDim **) 0; }

x_slot = y_slot = z_slot = t_slot = -1;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void VarInfo::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "var = ";

if ( var )  out << var << '\n';
else        out << "(nul)\n";

out << prefix << "name = ";

if ( name.length() > 0 )  out << '\"' << name << '\"';
else                      out << "(nul)";

if ( level.length() > 0 )  out << '\"' << level << '\"';
else                       out << "(nul)";

if ( units.length() > 0 )  out << '\"' << units << '\"';
else                       out << "(nul)";

out << prefix << "AccumTime = " << AccumTime;

out << "\n";

out << prefix << "Ndims = " << Ndims;

if ( Dims )  {

   int j;

   out << "[ ";

   for (j=0; j<Ndims; ++j)  out << Dims[j]->size() << ' ';

   out << "]\n";

}

out << prefix << "x_slot = " << x_slot << "\n";
out << prefix << "y_slot = " << y_slot << "\n";
out << prefix << "z_slot = " << z_slot << "\n";
out << prefix << "t_slot = " << t_slot << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void VarInfo::assign(const VarInfo & i)

{

clear();

var = i.var;

name = i.name;

level = i.level;

units = i.units;

AccumTime = i.AccumTime;

Ndims = i.Ndims;

x_slot = i.x_slot;
y_slot = i.y_slot;
z_slot = i.z_slot;
t_slot = i.t_slot;

if ( i.Dims )  {

   Dims = new NcDim * [i.Ndims];

   for (int j=0; j<(i.Ndims); ++j)  Dims[j] = i.Dims[j];

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////



