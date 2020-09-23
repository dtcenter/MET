// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_log.h"
#include "vx_util.h"

#include "insitu_nc_file.h"


////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////

const double MISSING_DATA_VALUE = -9999.0;
const int MED_EDR_GRIB_CODE = 200;
const int MAX_EDR_GRIB_CODE = 200;
const double FEET_TO_M = 0.3048;


int main(int argc, char * argv [])
{
  static const string method_name = "insitu_nc_to_ascii::main()";
  
  // Check the command line and extract the arguments

  ConcatString program_name = (string)get_short_name(argv[0]);

  if (argc != 3)
  {
    cerr << "\n\n   usage: " << program_name << " <insitu nc file path> <output file path>\n\n";

    exit(1);
  }

  const char * const input_filepath = argv[1];
  const char * const output_filepath = argv[2];

  // Open the input file

  InsituNcFile input_file;
  
  if (!input_file.open(input_filepath))
    exit(1);
  
  // Open the output file

  FILE *output_file;
  if ((output_file = met_fopen(output_filepath, "w")) == 0)
  {
//    mlog << Error << "\n" << method_name << " -> "
//	 << "error opening output file" << output_filepath << endl;
    cerr << "Error opening output file: " << output_filepath << endl;

    exit(1);
  }
  
  // Extract the records from the netCDF file and write them to the
  // output file

  string aircraft_id;
  time_t time_obs;
  double latitude;
  double longitude;
  double altitude;
  double qc_confidence;
  double med_edr;
  double max_edr;
  
  while (input_file.getNextRecord(aircraft_id, time_obs,
				  latitude, longitude, altitude,
				  qc_confidence, med_edr, max_edr))
  {
    // Construct the time string

    struct tm *time_struct = gmtime(&time_obs);
    if (time_struct == 0)
    {
//      mlog << Error << "\n" << method_name << " -> "
//	   << "error converting time value to time structure" << endl;
      cerr << "Error converting time value to time structure" << endl;
      fclose(output_file);
      exit(1);
    }

    char time_obs_string[80];
    
    snprintf(time_obs_string, sizeof(time_obs_string), 
         "%04d%02d%02d_%02d%02d%02d",
	    time_struct->tm_year + 1900, time_struct->tm_mon + 1,
	    time_struct->tm_mday,
	    time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);
    
    // Write the observations.

    fprintf(output_file, "MEDEDR %10s %s %10.4f %10.4f %8.2f %3d %8.2f %8.2f %8.2f %8.2f\n",
	    aircraft_id.c_str(), time_obs_string,
	    latitude, longitude, altitude * FEET_TO_M,
	    MED_EDR_GRIB_CODE, MISSING_DATA_VALUE, altitude,
	    qc_confidence, med_edr);
    
    fprintf(output_file, "MAXEDR %10s %s %10.4f %10.4f %8.2f %3d %8.2f %8.2f %8.2f %8.2f\n",
	    aircraft_id.c_str(), time_obs_string,
	    latitude, longitude, altitude * FEET_TO_M,
	    MAX_EDR_GRIB_CODE, MISSING_DATA_VALUE, altitude,
	    qc_confidence, max_edr);
    
  }
  fclose(output_file);  
  return 0;
}


////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////


