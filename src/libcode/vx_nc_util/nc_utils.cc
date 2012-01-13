////////////////////////////////////////////////////////////////////////

using namespace std;

#include <string.h>

#include <netcdf.hh>

#include "nc_utils.h"
#include "util_constants.h"

////////////////////////////////////////////////////////////////////////

static const char  level_att_name     [] = "level";
static const char  units_att_name     [] = "units";

////////////////////////////////////////////////////////////////////////

bool get_var_att(NcVar *var, const ConcatString &att_name,
                 ConcatString &att_val) {
   int i, n;
   NcAtt *att = (NcAtt *) 0;
   bool status = false;
   
   // Initialize
   att_val.clear();

   n = var->num_atts();

   // Loop through the attributes looking for a match
   for(i=0; i<n; i++) {

      att = var->get_att(i);

      // Look for a match
      if(strcmp(att_name, att->name()) == 0) {
         att_val << att->as_string(0);
         status = true;
         break;
      }
   }
   
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_var_units(NcVar *var, ConcatString &att_val) {
  
   return(get_var_att(var, "units_att_name", att_val));
}

////////////////////////////////////////////////////////////////////////

bool get_var_level(NcVar *var, ConcatString &att_val) {

   return(get_var_att(var, "level_att_name", att_val));
}

////////////////////////////////////////////////////////////////////////

int get_int_var(NcFile * nc, const char * var_name, int index) {
   int k;
   NcVar * var = (NcVar *) 0;

   var = nc->get_var(var_name);

   var->set_cur(index);

   var->get(&k, 1);

   return(k);
}

////////////////////////////////////////////////////////////////////////

bool args_ok(const LongArray & a) {
   int j, k;

   for (j=0; j<(a.n_elements()); ++j)  {
     
      k = a[j];

      if ( (k < 0) && (k != vx_data2d_star) ) return (false);
   }

   return(true);
}

////////////////////////////////////////////////////////////////////////
