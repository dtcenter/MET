// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   swinging_door.cc
//
//   Description:
//      Implement the swinging door algorithm as described in "Swinging
//      Door Trending: Adaptive Trend Recording?" by E.H. Bristol
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    08-19-14  Rehak          New
//   001    04-27-154 Halley Gotway  List and format output files
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "vx_nc_util.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_log.h"
#include "vx_time_series.h"

#include "met_nc_file.h"

////////////////////////////////////////////////////////////////////////

// Constants
static const char *program_name = "swinging_door";

////////////////////////////////////////////////////////////////////////

// Variables for command line arguments
static string ncfile_arg;
static double error_arg;
static int grib_code_arg;
static string message_type_arg;
static string station_id_arg;

////////////////////////////////////////////////////////////////////////

static bool process_file();
static bool run_algorithm(const vector< SDObservation > &obs,
                          vector< SDObservation > &compressed_obs);
static void usage();
static bool write_observations(const vector< SDObservation > &observations,
                               const string &file_path);
static bool write_ramps(const vector< SDObservation > &observations,
                        const string &file_path);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  CommandLine cline;

  // Set handler to be called for memory allocation error

  set_new_handler(oom);

  // Check for zero arguments

  if (argc == 1)
    usage();

  // Parse the command line into tokens

  cline.set(argc, argv);

  // Set the usage function

  cline.set_usage(usage);

  // Parse the command line

  cline.parse();

  // Make sure all of the command line arguments are there.  If so,
  // parse them.

  if (cline.n() != 5)
    usage();

  ncfile_arg = cline[0];
  error_arg = atof(cline[1].c_str());
  grib_code_arg = atoi(cline[2].c_str());
  message_type_arg = cline[3];
  station_id_arg = cline[4];

  // Now, run the algorithm

  if (!process_file())
    return -1;

  return 0;
}

////////////////////////////////////////////////////////////////////////

bool process_file()
{
  // Read in the input data

  MetNcFile nc_file(ncfile_arg);

  vector< SDObservation > observations;

  if (!nc_file.readFile(grib_code_arg, station_id_arg, message_type_arg,
                        observations))
    return false;

  // Write the raw observations to the output file

  if (!write_observations(observations, "obs.txt"))
    return false;

  // Run  the algorithm

  vector< SDObservation > compressed_observations;

  if (!run_algorithm(observations, compressed_observations))
    return false;

  // Write the compressed observations to the output file

  if (!write_observations(compressed_observations,
                          "compressed_obs.txt"))
    return false;

  // Write the compressed observations as ramps

  if (!write_ramps(compressed_observations, "ramps.txt"))
    return false;

  return true;
}


////////////////////////////////////////////////////////////////////////

bool run_algorithm(const vector< SDObservation > &observations,
                   vector< SDObservation > &compressed_observations)
{
  // Run the algorithm

  vector< pair< SDObservation, SDObservation > > ramps;

  if (!compute_swinging_door_ramps(observations, error_arg, ramps))
    return false;

  // Create the compressed observations from the ramps

  vector< pair< SDObservation, SDObservation > >::const_iterator ramp;
  for (ramp = ramps.begin(); ramp != ramps.end(); ++ramp)
  {
    compressed_observations.push_back(ramp->first);
    compressed_observations.push_back(ramp->second);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////

void usage()
{
  cout << "\nUsage: "
       << program_name << "\n"
       << "\tnetcdf_file\n"
       << "\terror_value\n"
       << "\tgrib_code\n"
       << "\tmessage_type\n"
       << "\tstation_id\n\n"

       << "\twhere\t\"netcdf_file\" is the NetCDF file containing "
       << "the raw observations (required).\n"

       << "\t\t\"error_value\" is the error value (E) to use when "
       << "computing the corridors (required).\n"

       << "\t\t\"grib_code\" is the grib code of the field to use "
       << "(required).\n"

       << "\t\t\"message_type\" is the message type of the observations "
       << "to use (required).\n"

       << "\t\t\"station_id\" is the station identifier of the "
       << "observations to use (required).\n"

       << flush;

  exit(1);
}

////////////////////////////////////////////////////////////////////////

bool write_observations(const vector< SDObservation > &observations,
                        const string &file_path)
{
  static const string method_name = "write_observations()";

  mlog << Debug(2)
       << "Writing: " << file_path << "\n";

  // Open the output file

  FILE *output_file;

  if ((output_file = fopen(file_path.c_str(), "w")) == 0)
  {
    mlog << Error << "\n" + method_name + " -> "
         << "Error opening output file: " << file_path << ".\n\n";
    return false;
  }

  // Write the header line

  fprintf(output_file, "valid_time,unix_time,value\n");

  // Write the observations

  vector< SDObservation >::const_iterator obs;
  for (obs = observations.begin(); obs != observations.end(); ++obs)
    fprintf(output_file, "%s,%d,%f\n",
            unix_to_yyyymmdd_hhmmss(obs->getValidTime()).text(),
            (int) obs->getValidTime(), obs->getValue());

  // Close the output file

  fclose(output_file);

  return true;
}

////////////////////////////////////////////////////////////////////////

bool write_ramps(const vector< SDObservation > &observations,
                 const string &file_path)
{
  static const string method_name = "write_ramps()";

  mlog << Debug(2)
       << "Writing: " << file_path << "\n";

  // Open the output file

  FILE *output_file;

  if ((output_file = fopen(file_path.c_str(), "w")) == 0)
  {
    mlog << Error << "\n" + method_name + " -> "
         << "Error opening output file: " << file_path << ".\n\n";
    return false;
  }

  // Write the header line

  fprintf(output_file, "start_time,end_time,run_secs,rise,slope\n");

  // Write the observations

  if (observations.size() % 2 != 0)
  {
    mlog << Warning << "\n" + method_name + " -> "
         << "There should be an even number of compressed observations. "
         << "An odd number were found, so there is probably a programming error. "
         << "Printing out the ramps, ignoring the last observation.\n\n";
  }

  size_t num_ramps = observations.size() / 2;

  for (size_t i = 0; i < num_ramps; ++i)
  {
    size_t start_obs = i * 2;
    size_t end_obs = start_obs + 1;

    int run_secs = observations[end_obs].getValidTime() - observations[start_obs].getValidTime();
    double rise = observations[end_obs].getValue() - observations[start_obs].getValue();
    double slope = rise / (double)run_secs;

    fprintf(output_file, "%s,%s,%d,%f,%f\n",
            observations[start_obs].getValidTimeString().c_str(),
            observations[end_obs].getValidTimeString().c_str(),
            run_secs, rise, slope);
  }

  // Close the output file

  fclose(output_file);

  return true;
}

////////////////////////////////////////////////////////////////////////
