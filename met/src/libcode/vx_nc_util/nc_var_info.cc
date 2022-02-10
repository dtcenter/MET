

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
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

#include "nc_utils.h"
#include "nc_var_info.h"
#include "vx_math.h"
#include "vx_log.h"
#include "vx_cal.h"

unixtime get_att_value_unixtime(const NcAtt *att) {
   ConcatString s;
   unixtime time_value = -1;
   switch ( GET_NC_TYPE_ID_P(att) )  {
      case NC_INT64:
      case NC_INT:
         time_value = get_att_value_int(att);
         break;

      case NC_CHAR:
         get_att_value_chars(att, s);
         // 20120410_120000 VS. 1333929600
         if (0 > s.find('_') && 0 > s.find('-'))
            time_value = string_to_unixtime(s.c_str());
         else
            time_value = yyyymmdd_hhmmss_to_unix(s.c_str());
         break;

      default:
         mlog << Warning << "get_att_value_unixtime() The attribute type ("
              << GET_NC_TYPE_NAME_P(att) << ") is not supported\n";
         break;
   }   //  switch
   return time_value;
}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class NcVarInfo
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

ValidTime = (unixtime) 0;

InitTime = (unixtime) 0;

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

ConcatString time_str;

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

time_str = unix_to_yyyymmdd_hhmmss(ValidTime);
out << prefix << "ValidTime = " << time_str << " (" << ValidTime << ")\n";

time_str = unix_to_yyyymmdd_hhmmss(InitTime);
out << prefix << "InitTime  = " << time_str << " (" << InitTime  << ")\n";

out << prefix << "AccumTime = " << AccumTime;

out << "\n";

out << prefix << "Ndims = " << Ndims;

if ( Dims )  {

   int j;

   out << "[ ";

   for (j=0; j<Ndims; ++j)  out << Dims[j]->getSize() << ' ';

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


int NcVarInfo::lead_time() const

{

unixtime dt = ValidTime - InitTime;

return ( (int) dt );

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

ValidTime = i.ValidTime;

InitTime = i.InitTime;

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


bool get_att_str(const NcVarInfo &info, const ConcatString att_name, ConcatString &att_value)

{

   NcVarAtt *att ;
   bool found = false;
   
   att_value.clear();
   
   att = get_nc_att(info.var, att_name, false);
   if (!IS_INVALID_NC_P(att)) {
      found = get_att_value_chars(att, att_value);
      if ( !found)  {
         // Check for the correct type
         if ( GET_NC_TYPE_ID_P(att) != NcType::nc_CHAR ) {
         
            mlog << Error << "\nget_att_str(const NcVarInfo &, const ConcatString &, ConcatString &) -> "
                   << "attribute \"" << att_name << "\" should be a string.\n\n";
         
            exit ( 1 );
         }
      }
   }
   if (att) delete att;

   //
   //  done
   //

   return ( found );

}


////////////////////////////////////////////////////////////////////////


bool get_att_int(const NcVarInfo &info, const ConcatString att_name, int &att_value)

{

   att_value = bad_data_int;
   
   NcVarAtt *att = get_nc_att(info.var, att_name, false);
   bool found = IS_VALID_NC_P(att);
   if (found) {
      att_value = get_att_value_int(att);
   
      // Check for the correct type
      nc_type att_type = GET_NC_TYPE_ID_P(att);
      if ( att_type != NcType::nc_INT && att_type != NcType::nc_INT64
           && att_type != NcType::nc_SHORT && att_type != NcType::nc_BYTE ) {
   
         mlog << Error << "\nget_att_int(const NcVarInfo &, const ConcatString &, int &) -> "
              << "attribute \"" << att_name << "\" should be an integer.\n\n";
   
         exit ( 1 );
      }
   }
   if (att) delete att;
   
   //
   //  done
   //
   
   return ( found );

}


////////////////////////////////////////////////////////////////////////


bool get_att_unixtime(const NcVarInfo &info, const ConcatString att_name, unixtime &att_value)

{

   att_value = (unixtime) bad_data_int;

   NcVarAtt *att = get_nc_att(info.var, att_name, false);
   bool found = IS_VALID_NC_P(att);
   if( found ) att_value = get_att_value_unixtime(att);

   if (att) delete att;
   
   //
   //  done
   //
   
   return ( found );

}


////////////////////////////////////////////////////////////////////////


