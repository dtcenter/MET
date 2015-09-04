

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "data2d_factory.h"

#include "mtd_read_data.h"


////////////////////////////////////////////////////////////////////////


void mtd_read_data(MtdConfigInfo & config, VarInfo & varinfo, const StringArray & filenames, MtdFloatFile & raw)

{

if ( filenames.n() < 2 )  {

   cerr << "\n\n  mtd_read_data() -> need at least 2 data files!\n\n";

   exit ( 1 );

}

int j;
// Dictionary * dict = 0;
Met2dDataFile * data_2d_file = 0;
Met2dDataFileFactory factory;
DataPlane plane;
unixtime * valid = 0;

   //
   //  read the first file
   //

// dict = config.conf.lookup_dictionary(conf_key_fcst);
// 
// ft = parse_conf_file_type(dict);

valid = new unixtime [filenames.n()];

   //
   //  read the files
   //

for (j=0; j<(filenames.n()); ++j)  {

   cout << "mtd_read_data() -> processing file \"" << filenames[j] << "\"\n" << flush;

   data_2d_file = factory.new_met_2d_data_file(filenames[j]);

   if ( ! data_2d_file->data_plane(varinfo, plane) )  {

      cerr << "\n\n  mtd_read_data() -> unable to get data plane at time " << j << "\n\n";

      exit ( 1 );

   }

   if ( ! data_2d_file->data_plane(varinfo, plane) )  {

      cerr << "\n\n  mtd_read_data() -> unable to get data plane at time " << j << "\n\n";

      exit ( 1 );

   }

   valid[j] = plane.valid();

   if ( j == 0 )  {

      raw.set_size(plane.nx(), plane.ny(), filenames.n());

      raw.set_start_time(valid[0]);

      raw.set_grid(data_2d_file->grid());

   }

   raw.put(plane, j);

   delete data_2d_file;  data_2d_file = 0;

}   //  for j


   //
   //  check the time intervals
   //

unixtime dt_start, dt;

dt_start = valid[1] - valid[0];

for (j=2; j<(filenames.n()); ++j)  {

   dt = valid[j] - valid[j - 1];

   if ( dt != dt_start )  {

      cerr << "\n\n  mtd_read_data() -> file time increments are not constant!\n\n";

      exit ( 1 );

   }

}

   //
   //  load up the rest of the MtdFloatFile class members
   //

raw.set_delta_t((int) dt_start);

raw.set_filetype(mtd_file_raw);

   //
   //  data range
   //

raw.calc_data_minmax();

   //
   //  done
   //

if ( valid )  { delete [] valid;  valid = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


