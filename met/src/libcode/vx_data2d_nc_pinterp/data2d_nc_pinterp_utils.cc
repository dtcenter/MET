

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "util_constants.h"
#include "data2d_nc_pinterp_utils.h"


////////////////////////////////////////////////////////////////////////


int get_int_var(NcFile * nc, const char * var_name, int index)

{

int k;
NcVar * var = (NcVar *) 0;


var = nc->get_var(var_name);

var->set_cur(index);

var->get(&k, 1);


return ( k );

}


////////////////////////////////////////////////////////////////////////


bool args_ok(const LongArray & a)

{

int j, k;

for (j=0; j<(a.n_elements()); ++j)  {

   k = a[j];

   if ( (k < 0) && (k != vx_data2d_star) )  return ( false );

}

return ( true );

}


////////////////////////////////////////////////////////////////////////

