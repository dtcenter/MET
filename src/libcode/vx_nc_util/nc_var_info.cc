

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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

#include "nc_var_info.h"
#include "vx_math.h"
#include "vx_log.h"
#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class NcNcVarInfo
   //


////////////////////////////////////////////////////////////////////////


NcVarInfo::NcVarInfo()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


NcVarInfo::~NcVarInfo()

{

clear();

}


////////////////////////////////////////////////////////////////////////


NcVarInfo::NcVarInfo(const NcVarInfo & i)

{

init_from_scratch();

assign(i);

}


////////////////////////////////////////////////////////////////////////


NcVarInfo & NcVarInfo::operator=(const NcVarInfo & i)

{

if ( this == &i )  return ( * this );

assign(i);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void NcVarInfo::init_from_scratch()

{

Dims = (NcDim **) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void NcVarInfo::clear()

{

var = (NcVar *) 0;   //  don't delete

name.clear();

name_att.clear();

long_name_att.clear();

level_att.clear();

units_att.clear();

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


void NcVarInfo::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "var = ";

if ( var )  out << var << '\n';
else        out << "(nul)\n";

out << prefix << "name = ";

if ( name.length() > 0 )          out << '\"' << name << '\"';
else                              out << "(nul)";

if ( name_att.length() > 0 )      out << '\"' << name_att << '\"';
else                              out << "(nul)";

if ( long_name_att.length() > 0 ) out << '\"' << long_name_att << '\"';
else                              out << "(nul)";

if ( level_att.length() > 0 )     out << '\"' << level_att << '\"';
else                              out << "(nul)";

if ( units_att.length() > 0 )     out << '\"' << units_att << '\"';
else                              out << "(nul)";

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


void NcVarInfo::assign(const NcVarInfo & i)

{

clear();

var = i.var;

name = i.name;

name_att = i.name_att;

long_name_att = i.long_name_att;

level_att = i.level_att;

units_att = i.units_att;

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


   //
   //  End code for class NcNcVarInfo
   //


////////////////////////////////////////////////////////////////////////


bool get_att_str(const NcVarInfo &info, const ConcatString &att_name, ConcatString &att_value)

{

int j, n;
NcAtt * att = (NcAtt *) 0;
bool found = false;

n = info.var->num_atts();
att_value.clear();

for (j=0; j<n; ++j)  {

   att = info.var->get_att(j);

   if ( strcmp(att_name, att->name()) == 0 )  {

      // Check for the correct type
      if ( att->type() != ncChar ) {

         mlog << Error << "\nget_att_str(const NcVarInfo &, const ConcatString &, ConcatString &) -> "
              << "attribute \"" << att_name << "\" should be a string.\n\n";

         exit ( 1 );
      }
     
      att_value = att->as_string(0);
      found = true;

      break;

   }

}

   //
   //  done
   //

return ( found );

}


////////////////////////////////////////////////////////////////////////


bool get_att_int(const NcVarInfo &info, const ConcatString &att_name, int &att_value)

{

int j, n;
NcAtt * att = (NcAtt *) 0;
bool found = false;

n = info.var->num_atts();
att_value = bad_data_int;

for (j=0; j<n; ++j)  {

   att = info.var->get_att(j);

   if ( strcmp(att_name, att->name()) == 0 )  {

      // Check for the correct type
      if ( att->type() != ncInt ) {

         mlog << Error << "\nget_att_int(const NcVarInfo &, const ConcatString &, int &) -> "
              << "attribute \"" << att_name << "\" should be an integer.\n\n";

         exit ( 1 );
      }
     
      att_value = att->as_int(0);
      found = true;

      break;

   }

}

   //
   //  done
   //

return ( found );

}


////////////////////////////////////////////////////////////////////////


bool get_att_unixtime(const NcVarInfo &info, const ConcatString &att_name, unixtime &att_value)

{

int j, n;
NcAtt * att = (NcAtt *) 0;
bool found = false;

n = info.var->num_atts();

att_value = (unixtime) bad_data_int;

for (j=0; j<n; ++j)  {

   att = info.var->get_att(j);

   if ( strcmp(att_name, att->name()) == 0 )  {

      found = true;

      break;

   }

}

if ( !found )  return ( false );

   // Check the type

ConcatString s;

switch ( att->type() )  {

   case ncInt:
      att_value = (unixtime) (att->as_int(0));
      break;

   case ncChar:
      s = att->as_string(0);
      att_value = string_to_unixtime(s);
      break;

   default:
         mlog << Error << "\nget_att_unixtime(const NcVarInfo &, const ConcatString &, unixtime &) -> "
              << "attribute \"" << att_name << "\" should be an integer or a string.\n\n";
         exit ( 1 );
      break;

}   //  switch


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


