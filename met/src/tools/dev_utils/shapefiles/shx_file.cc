

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "vx_util.h"

#include "shx_file.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct ShxRecord
   //


////////////////////////////////////////////////////////////////////////


void ShxRecord::set(unsigned char * buf)

{

int * i = (int *) buf;

shuffle_4(buf);
shuffle_4(buf + 4);

offset            = i[0];
content_length_16 = i[1];

content_length_bytes = 2*content_length_16;

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void ShxRecord::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "offset         = " << offset               << "\n";
out << prefix << "content_length = " << content_length_bytes << "\n";

   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////




