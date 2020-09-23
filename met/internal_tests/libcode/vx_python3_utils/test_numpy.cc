

////////////////////////////////////////////////////////////////////////


static const char script_name    [] = "np.py";

static const char variable_name  [] = "data";


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"

#include "python3_util.h"
#include "python3_script.h"
#include "python3_numpy.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

Python3_Script script(script_name);
Python3_Numpy np;

cout << "\n\n=======================================================\n\n";

np.set(script, variable_name);

np.dump(cout);

   //
   //  at this point, we're assuming the data is longs
   //

int j;
long * p = (long *) np.buffer();

for (j=0; j<(np.n_data()); ++j)  cout << ' ' << p[j];

cout << "\n";


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


