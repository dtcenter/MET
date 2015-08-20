

////////////////////////////////////////////////////////////////////////


static const char mtd_config_filename    [] = "../../../../data/config/MTDConfigDefault";

static const char local_config_filename  [] = "test_config";


static const char fcst_filename [] = "/scratch/bullock/files/arw_20100517_00I.nc";
static const char  obs_filename [] = "/scratch/bullock/files/obs_20100517_01L.nc";


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "mtd_config_info.h"
#include "mtd_file.h"


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

MtdConfigInfo config;

config.read_config(mtd_config_filename, local_config_filename);

// config.process_config(FileType_General_Netcdf, FileType_General_Netcdf);
config.process_config(FileType_NcMet, FileType_NcMet);

MtdFloatFile fcst_raw, obs_raw;
MtdFloatFile fcst_conv, obs_conv;
MtdIntFile fcst_mask, obs_mask;
MtdIntFile fcst_obj, obs_obj;


if ( ! fcst_raw.read(fcst_filename) )  {

   cerr << "\n\n  " << program_name << ": unable to read fcst file \"" << fcst_filename << "\"\n\n";

   exit ( 1 );

}

if ( ! obs_raw.read(obs_filename) )  {

   cerr << "\n\n  " << program_name << ": unable to read obs file \"" << obs_filename << "\"\n\n";

   exit ( 1 );

}


cout << "\n  fcst conv radius = " << (config.fcst_conv_radius) << "\n";
cout << "\n   obs conv radius = " << (config.obs_conv_radius) << "\n";

cout << "\n\n  fcst threshold:\n";
config.fcst_conv_thresh.dump(cout);

cout << "\n\n  obs threshold:\n";
config.obs_conv_thresh.dump(cout);





   //
   //  done
   //

return ( 0 );

}

////////////////////////////////////////////////////////////////////////

