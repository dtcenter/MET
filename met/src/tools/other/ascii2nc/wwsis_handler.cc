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

#include "vx_log.h"
#include "vx_math.h"
#include "vx_util.h"

#include "wwsis_handler.h"


const string WwsisHandler::HEADER_TYPE = (string)"WWSIS";

const int WwsisHandler::GRIB_CODE = 0;
const string GRIB_NAME = (string)"WWSIS";

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class WwsisHandler
   //


////////////////////////////////////////////////////////////////////////


WwsisHandler::WwsisHandler(const string &program_name) :
  FileHandler(program_name)
{
  use_var_id = true;
  //do_monitor = true;
}


////////////////////////////////////////////////////////////////////////


WwsisHandler::~WwsisHandler()
{
}


////////////////////////////////////////////////////////////////////////


bool WwsisHandler::isFileType(LineDataFile &ascii_file) const
{
  // Initialize the return value.

  bool is_file_type = true;
  
  // Read the first line from the file.  This should be the header line.
  // Check to see if it matches what we expect.

  DataLine dl;
  ascii_file >> dl;
  string line = dl.get_line();
  line = line.substr(0, line.length() - 1);
  
  if (line != "lat,lon,tz,step,watts,derate,tracking,tilt,azimuth")
    is_file_type = false;

  return is_file_type;
}
  

////////////////////////////////////////////////////////////////////////
// Private/Protected methods
////////////////////////////////////////////////////////////////////////

int WwsisHandler::_getYearFromFilename(const string &filename) const
{
  // Copy the filename so we can keep cutting away at it until we end up with
  // the year.

  string work_string = filename;
  
  // The files are named using the following convention:
  //    <datatype>_<latlon>_<year>_pv_<plant.capacity>MW_<data_frequency>_min_v3pt4.csv
  // We need to start from the end of the filename because the <datatype> can
  // contain an underscore.

  size_t underscore_pos;

  // Remove the "v3pt4"

  if ((underscore_pos = work_string.rfind("_")) == string::npos)
    return 0;
  work_string = work_string.substr(0, underscore_pos);

  // Remove the "min"

  if ((underscore_pos = work_string.rfind("_")) == string::npos)
    return 0;
  work_string = work_string.substr(0, underscore_pos);

  // Remove the data frequency

  if ((underscore_pos = work_string.rfind("_")) == string::npos)
    return 0;
  work_string = work_string.substr(0, underscore_pos);
  
  // Remove the plant capacity

  if ((underscore_pos = work_string.rfind("_")) == string::npos)
    return 0;
  work_string = work_string.substr(0, underscore_pos);
  
  // Remove the "pv", if it exists.  The "HA_pvwatts..." files don't have the
  // "pv" part.

  if ((underscore_pos = work_string.rfind("_")) == string::npos)
    return 0;
  if (work_string.substr(underscore_pos) == "_pv")
    work_string = work_string.substr(0, underscore_pos);
  
  // The next underscore will be the one before the year

  if ((underscore_pos = work_string.rfind("_")) == string::npos)
    return 0;
  work_string = work_string.substr(underscore_pos+1);
  
  return atoi(work_string.c_str());
}

////////////////////////////////////////////////////////////////////////

time_t WwsisHandler::_initValidTime(const string &filename) const
{
  // Pull the year out of the filename

  int year = _getYearFromFilename(filename);
  if (year == 0)
    return 0;
  
  // Set the initial valid time to Jan 1 of that year

  struct tm time_struct;
  memset(&time_struct, 0, sizeof(time_struct));
  
  time_struct.tm_year = year - 1900;
  
  // Add the time zone offset

  return timegm(&time_struct) + _timeZoneOffsetSecs;
}

////////////////////////////////////////////////////////////////////////

bool WwsisHandler::_readObservations(LineDataFile &ascii_file)
{
  DataLine data_line;
  static const string method_name = "WwsisHandler::_readObservations()";

  if (do_monitor) start_time = clock();
  
  // Read and save the header information

  if (!_readHeaderInfo(ascii_file))
    return false;
   
  // Initialize the data valid time from the file name

  time_t valid_time = _initValidTime(ascii_file.short_filename());
  if (valid_time == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "cannot extract beginning valid time from file name: "
         << ascii_file.short_filename() << ".\n\n";
    return false;
  }
  
  // Process the observation lines

  while (ascii_file >> data_line)
  {
    // Make sure that the line contains the correct number of tokens

    if (data_line.n_items() != 1)
    {
      mlog << Error << "\n" << method_name << " -> "
           << "line number " << data_line.line_number()
           << " does not have the correct number of columns  (1).\n\n";
      return false;
    }

    // Add the observation

    _addObservations(Observation(HEADER_TYPE, (string)"WWSIS",
                                 valid_time,
                                 _stationLat, _stationLon,
                                 0,
                                 na_str,
                                 GRIB_CODE,
                                 0.0, 0.0,
                                 atof(data_line[0]),
                                 GRIB_NAME));
     
    // Increment the valid time

    valid_time += _stepSecs;
     
  }
  
  if (do_monitor) {
    mlog << Debug(3) << " PERF: " << method_name << " "
         << (int)(clock()-start_time)/double(CLOCKS_PER_SEC) << " seconds for reading obs" << "\n";
  }
   
  return true;
}
  
////////////////////////////////////////////////////////////////////////

bool WwsisHandler::_readHeaderInfo(LineDataFile &ascii_file)
{
  static const string method_name = "WwsisHandler::_readHeaderInfo()";
  
  DataLine data_line;
  data_line.set_delimiter(",");
  
  // The first line of the file contains text specifying the header line
  // fields.  We can skip this line.

  if (!(ascii_file >> data_line))
  {
    mlog << Error << "\n" << method_name << " -> "
         << "error reading header line from input ASCII file \""
         << ascii_file.filename() << "\"\n\n";

    return false;
  }

  // The second line gives the station, etc information for this file.

  if (!(ascii_file >> data_line))
  {
    mlog << Error << "\n" << method_name << " -> "
         << "error reading station id line from input ASCII file \""
         << ascii_file.filename() << "\"\n\n";

    return false;
  }

  // Make sure we have the right number of tokens

  if (data_line.n_items() != 9)
  {
    mlog << Error << "\n" << method_name << " -> "
         << "error reading station id line from input ASCII file \""
         << ascii_file.filename() << "\"\n\n";

    return false;
  }
  
  _stationLat = atof(data_line[0]);
  _stationLon = atof(data_line[1]);
  _timeZoneOffsetSecs = atoi(data_line[2]) * 3600;
  _stepSecs = atoi(data_line[3]) * 60;
  _watts = atof(data_line[4]);
  _derate = atof(data_line[5]);
  _tracking = data_line[6];
  _tilt = atof(data_line[7]);
  _azimuth = atof(data_line[8]);
  
  // The next line just has "ac" on it and needs to be skipped before
  // we process the rest of the file

  if (!(ascii_file >> data_line))
  {
    mlog << Error << "\n" << method_name << " -> "
         << "error reading \"ac\" from input ASCII file \""
         << ascii_file.filename() << "\"\n\n";

    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////
