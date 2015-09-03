

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "data2d_factory.h"

#include "mtd_read_data.h"


////////////////////////////////////////////////////////////////////////


void mtd_read_data(MtdConfigInfo & config, VarInfo & varinfo, const StringArray & filenames, MtdFloatFile & data)

{

int j;
// Dictionary * dict = 0;
Met2dDataFile * data_2d_file = 0;
Met2dDataFileFactory factory;
DataPlane plane;

   //
   //  read the first file
   //

// dict = config.conf.lookup_dictionary(conf_key_fcst);
// 
// ft = parse_conf_file_type(dict);

data_2d_file = factory.new_met_2d_data_file(filenames[0]);

if ( ! data_2d_file->data_plane(varinfo, plane) )  {

   cerr << "\n\n  mtd_read_data() -> unable to get data plane\n\n";

   exit ( 1 );

}

plane.dump(cout);

   //
   //  read the resto of the files
   //

for (j=1; j<(filenames.n()); ++j)  {   //  j starts at one, here




}   //  for j


   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


