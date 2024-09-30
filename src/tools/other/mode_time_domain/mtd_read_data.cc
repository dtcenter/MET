// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "data2d_factory.h"

#include "mtd_read_data.h"

#include "num_array.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


vector<unixtime> mtd_read_data(MtdConfigInfo & config, VarInfo & varinfo,
                               const StringArray & filenames, MtdFloatFile & raw)

{

static const char *method_name = "mtd_read_data() -> ";

if ( filenames.n() < 2 )  {

   mlog << Error << "\n" << method_name
        << "need at least 2 data files!\n\n";

   exit ( 1 );

}

int j, k;
Met2dDataFile * data_2d_file = 0;
Met2dDataFileFactory factory;
DataPlane plane;
vector<unixtime> valid_times;
   //
   //  read the files
   //

for (j=0; j<(filenames.n()); ++j)  {

   mlog << Debug(2) << method_name
        << "processing file: " << filenames[j] << "\n";

   data_2d_file = factory.new_met_2d_data_file(filenames[j].c_str(), varinfo.file_type());

   if ( ! data_2d_file->data_plane(varinfo, plane) )  {

      mlog << Error << "\n" << method_name
           << "unable to get data plane at time " << j << "\n\n";

      exit ( 1 );

   }

   if ( ! data_2d_file->data_plane(varinfo, plane) )  {

      mlog << Error << "\n" << method_name
           << "unable to get data plane at time " << j << "\n\n";

      exit ( 1 );

   }

   valid_times.push_back(plane.valid());

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

    //  store the valid times vector into the raw data for later use in do_2d_txt_output()
 
 raw.init_actual_valid_times(valid_times);

   //
   //  check the time intervals for consistency
   //  Store the time differences between succesive valid times in an array
   //  See if differences are constant or not, and if not see if all diffs are months
   //

unixtime dt_start;
vector<unixtime> dtVector; 

dt_start = valid_times[1] - valid_times[0];
dtVector.push_back(dt_start);

for (size_t k=2; k<valid_times.size(); ++k) {
  dtVector.push_back(valid_times[k] - valid_times[k - 1]);
}

bool variableTimeIncs = false;
for (size_t k=0; k<dtVector.size(); ++k) {
  if (variableTimeIncs) {
     break;
  }
  for (size_t k2=k+1; k2<dtVector.size(); ++k2) {
    if ( dtVector[k] != dtVector[k2])  {
      variableTimeIncs = true;
      break;
    }
  }
}
if (variableTimeIncs) {
  // compute the mode and use it as the actual delta, by storing it to dt_start
  NumArray na;
  for (size_t k=0; k<dtVector.size(); ++k) {
    na.add((double)dtVector[k]);
   }   
   dt_start = (unixtime)na.mode();

   // test if the differences are all months (in seconds)
   bool isMonths = true;
   int secondsPerDay = 24*3600;
   for (size_t k=0; k<dtVector.size(); ++k) {
      int days = dtVector[k]/secondsPerDay;
      if (days != 28 && days != 29 && days != 30 && days != 31) {
         isMonths = false;
         break;
      }
   }

   if (isMonths) {
     mlog << Debug(1) << "File time increments are months (not constant), use MODE of the increments, mode="
          << dt_start << " seconds = " << dt_start/(24*3600) << " days\n\n";
   } else {
      // compute some measures that might be used to exit with an error, for now just show them to the user and go on
      double mean, var, svar;
      na.compute_mean_variance(mean, var);
      svar = sqrt(var);
      unixtime umean = (unixtime)mean;
      unixtime uvar = (unixtime)var;
      unixtime suvar = (unixtime)svar;
      mlog << Warning << "\n" << method_name
           << "file time increments are not constant, could be problematic\n\n";
      mlog << Warning << "\n" << method_name
           << "using MODE of the increments, mode=" << dt_start << "\n\n";
      mlog << Warning << "\n" << method_name
           << "Time increment properties: mean=" << umean << " variance=" << uvar
           << " sqrt(var)=" << suvar << "\n\n";
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

return valid_times;

}


////////////////////////////////////////////////////////////////////////


