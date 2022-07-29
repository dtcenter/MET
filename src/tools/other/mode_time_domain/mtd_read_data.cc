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

#include "data2d_factory.h"

#include "mtd_read_data.h"

#include "num_array.h"

////////////////////////////////////////////////////////////////////////


void mtd_read_data(MtdConfigInfo & config, VarInfo & varinfo,
                   const StringArray & filenames, MtdFloatFile & raw,
		   unixtime valid_times[])

{

if ( filenames.n() < 2 )  {

   mlog << Error << "\n\n  mtd_read_data() -> need at least 2 data files!\n\n";

   exit ( 1 );

}

int j, k;
Met2dDataFile * data_2d_file = 0;
Met2dDataFileFactory factory;
DataPlane plane;

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
   //  check the time intervals for consistency
   //  Store the time differences between succesive valid times in an array
   //  See if differences are constant or not, and if not see if all diffs are months
   //

unixtime dt_start, dt, *dtArray;
int numDt = filenames.n() - 1;
dtArray = new unixtime [numDt];

dt_start = valid_times[1] - valid_times[0];
dtArray[0] = dt_start;

for (j=2; j<(filenames.n()); ++j)  {
   dtArray[j - 1] = dt = valid_times[j] - valid_times[j - 1];
}

bool variableTimeIncs = false;
for (j=0; j<numDt; ++j) {
   if (variableTimeIncs) {
      break;
   }
   for (k=j+1; k<numDt; ++k) {
      if ( dtArray[j] != dtArray[k])  {
         variableTimeIncs = true;
	 break;
      }
   }
}
if (variableTimeIncs) {
   // compute the mode and use it as the actual delta, by storing it to dt_start
   NumArray na;
   for (j=0; j<numDt; ++j) {
      na.add((double)dtArray[j]);
   }	     
   dt_start = (unixtime)na.mode();

   // test if the differences are all months (in seconds)
   bool isMonths = true;
   int secondsPerDay = 24*3600;
   for (j=0; j<numDt; ++j) {
      int days = dtArray[j]/secondsPerDay;
      if (days != 28 &&	days != 29 && days != 30 && days != 31) {
	 isMonths = false;
	 break;
      }
   }

   if (isMonths) {
     mlog << Warning << "\n\n mtd_read_data() -> file time increments are months (not constant), use MODE of the increments mode=" << dt_start << " seconds = " << dt_start/(24*3600) << " days\n\n";
   } else {
      // compute some measures that might be used to exit with an error, for now just show them to the user and go on
      double mean, var, svar;
      na.compute_mean_variance(mean, var);
      svar = sqrt(var);
      unixtime umean = (unixtime)mean;
      unixtime uvar = (unixtime)var;
      unixtime suvar = (unixtime)svar;
      mlog << Warning << "\n\n mtd_read_data() -> file time increments are not constant, use MODE of the increments mode=" << dt_start << "\n";
      mlog << Warning << " mtd_read_data() -> mean=" << umean << " variance=" << uvar << " sqrt(var)=" << suvar << "\n\n";
   }
}
delete [] dtArray;
 
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

return;

}


////////////////////////////////////////////////////////////////////////


