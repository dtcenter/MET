// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

#include "data2d_factory.h"

#include "mtd_read_data.h"


////////////////////////////////////////////////////////////////////////


void mtd_read_data(MtdConfigInfo & config, VarInfo & varinfo,
                   const StringArray & filenames, MtdFloatFile & raw)

{

if ( filenames.n() < 2 )  {

   mlog << Error << "\n\n  mtd_read_data() -> need at least 2 data files!\n\n";

   exit ( 1 );

}

int j;
Met2dDataFile * data_2d_file = 0;
Met2dDataFileFactory factory;
DataPlane plane;
unixtime * valid_times = 0;



valid_times = new unixtime [filenames.n()];

   //
   //  read the files
   //

for (j=0; j<(filenames.n()); ++j)  {

   mlog << Debug(2)
        << "mtd_read_data() -> processing file \"" << filenames[j] << "\"\n";

   data_2d_file = factory.new_met_2d_data_file(filenames[j].c_str(), varinfo.file_type());

   if ( ! data_2d_file->data_plane(varinfo, plane) )  {

      mlog << Error << "\n\n  mtd_read_data() -> unable to get data plane at time " << j << "\n\n";

      exit ( 1 );

   }

   if ( ! data_2d_file->data_plane(varinfo, plane) )  {

      mlog << Error << "\n\n  mtd_read_data() -> unable to get data plane at time " << j << "\n\n";

      exit ( 1 );

   }

   valid_times[j] = plane.valid();

   if ( j == 0 )  {

      raw.set_size(plane.nx(), plane.ny(), filenames.n());

      raw.set_start_valid_time(valid_times[0]);

      raw.set_grid(data_2d_file->grid());

   }

   raw.set_lead_time(j, plane.lead());

   raw.put(plane, j);

   delete data_2d_file;  data_2d_file = 0;

}   //  for j

varinfo.set_valid(valid_times[0]);

   //
   //  check the time intervals
   //

unixtime dt_start, dt;

dt_start = valid_times[1] - valid_times[0];

for (j=2; j<(filenames.n()); ++j)  {

   dt = valid_times[j] - valid_times[j - 1];

   if ( dt != dt_start )  {

      mlog << Error << "\n\n  mtd_read_data() -> file time increments are not constant!\n\n";

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

if ( valid_times )  { delete [] valid_times;  valid_times = 0; }

return;

}


////////////////////////////////////////////////////////////////////////


