// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

// Commenting out functionality for Angstrom - We are pulling back on
// this until we have a use case.

////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <string>
#include <sstream>

#include "vx_log.h"
#include "vx_math.h"
#include "vx_util.h"

#include "aeronet_handler.h"

static const char *AERONET_NA_STR = "N/A";

const int AeronetHandler::NUM_HDR_COLS = 7;
const int AeronetHandler::NUM_OBS_COLS = 45;

const string AeronetHandler::HEADER_TYPE = "";  /////

const int AeronetHandler::AOT_GRIB_CODE = 129;

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class AeronetHandler
   //


////////////////////////////////////////////////////////////////////////


AeronetHandler::AeronetHandler(const string &program_name) :
  FileHandler(program_name)
{
}


////////////////////////////////////////////////////////////////////////


AeronetHandler::~AeronetHandler()
{
}


////////////////////////////////////////////////////////////////////////



bool AeronetHandler::isFileType(LineDataFile &ascii_file) const
{
  //
  // Initialize the return value.
  //
  bool is_file_type = false;

  //
  // Read the first, second, and third lines from the file.  We will
  // skip these line since we don't know how many tokens they will have.
  //
  DataLine dl;
  dl.set_delimiter(",");
  ascii_file >> dl;
  ascii_file >> dl;
  ascii_file >> dl;

  //
  // Read the fourth line in the file.  It should start with AOD Level.
  //
  ascii_file >> dl;
  string line = dl.get_line();
  line = line.substr(0, 9);

  if (strcmp(line.c_str(), "AOD Level") == 0)
    is_file_type = true;

  return is_file_type;
}


////////////////////////////////////////////////////////////////////////
// Private/Protected methods
////////////////////////////////////////////////////////////////////////

bool AeronetHandler::_readObservations(LineDataFile &ascii_file)
{
  DataLine data_line;

  //
  // Read and save the station name, latitude, longitude, and elevation.
  //

  if (!_readHeaderInfo(ascii_file))
    return false;

  //
  // Get the Level number from the fourth header line
  //

  ascii_file >> data_line;
  ConcatString dl_string(data_line[0]);
  StringArray tokens = dl_string.split(" ");
  string header_type = "AERONET" + string("_") + tokens[tokens.n_elements()-1];

  //
  // Get the field information from the fifth header line
  //

  ascii_file >> data_line;
  StringArray hdr_tokens;
  IntArray process_flag;

  hdr_tokens.parse_css(data_line.get_line());

  string aot = "AOT";
  //string angstrom = "Angstrom";

  for (int j = 0; j < hdr_tokens.n_elements(); j++)
  {
    string hdr_field = hdr_tokens[j];

    // Set the process_flag to 1 if AOT (or Angstrom - future code) is in the field name
    // otherwise set the process flag to 0

    if (hdr_field.find(aot) != string::npos)
    {
      process_flag.add(1);
    }
    //else if (hdr_field.find(angstrom) != string::npos)
    //{
    //  process_flag.add(1);
    //}
    else
    {
      process_flag.add(0);
    }
  }

  //
  // Process the observation lines
  //
  data_line.set_delimiter(",");
  while (ascii_file >> data_line)
  {
    //
    // Make sure that the line contains the correct number of tokens
    //

    if (data_line.n_items() != NUM_OBS_COLS)
    {
      mlog << Error << "\nAeronetHandler::_readObservations() -> "
	   << "line number " << data_line.line_number()
	   << " does not have the correct number of columns  ("
	   << NUM_OBS_COLS << ").\n\n";
      return false;
    }

    //
    // Pull the valid time from the data line
    //

    time_t valid_time = _getValidTime(data_line);

    if (valid_time == 0)
      return false;

    //
    // Save the desired observations from the line
    //
    for (int k = 0; k < process_flag.n_elements(); k++)
    {
      if (process_flag[k] == 1)
      {
	string hdr_field = hdr_tokens[k];
	size_t found_aot = hdr_field.find(aot);
	//int found_angstrom = hdr_field.find(angstrom);
	string height = "";

	if (found_aot != string::npos)
	{
	  height = hdr_field.substr((found_aot + 4), hdr_field.size() - 1);
	}

	//if (found_angstrom != string::npos)
	//{
	//  size_t found_dash = hdr_field.find("-");
	//  if (found_dash != string::npos)
	//  {
	//    height = hdr_field.substr(0, found_dash);
	//  }
	//}

	double dlevel = bad_data_double;
	double dheight = atoi(height.c_str());

	if(strcmp(data_line[k], AERONET_NA_STR) == 0) continue;

	_addObservations(Observation(header_type, _stationId,
					 valid_time,
					 _stationLat, _stationLon,
					 _stationAlt,
					 na_str,
					 AOT_GRIB_CODE,
					 dlevel, dheight,
					 atof(data_line[k])));
      }
    }
  } // end while

  return true;
}

////////////////////////////////////////////////////////////////////////

time_t AeronetHandler::_getValidTime(const DataLine &data_line) const
{
  //
  // Pull out the date information
  //

  ConcatString date_string(data_line[0]);
  StringArray dateTokens = date_string.split(":");
  string mday = dateTokens[0];
  string mon = dateTokens[1];
  string year = dateTokens[2];

  //
  // Pull out the time information
  //

  ConcatString time_string(data_line[1]);
  StringArray timeTokens = time_string.split(":");
  string hour = timeTokens[0];
  string min = timeTokens[1];
  string sec = timeTokens[2];

  //
  // Set up the time structure
  //

  struct tm time_struct;
  memset(&time_struct, 0, sizeof(time_struct));

  time_struct.tm_year = atoi(year.c_str()) - 1900;
  time_struct.tm_mon = atoi(mon.c_str()) - 1;
  time_struct.tm_mday = atoi(mday.c_str());
  time_struct.tm_hour = atoi(hour.c_str());
  time_struct.tm_min = atoi(min.c_str());
  time_struct.tm_sec = atoi(sec.c_str());

  return timegm(&time_struct);

}

////////////////////////////////////////////////////////////////////////

bool AeronetHandler::_readHeaderInfo(LineDataFile &ascii_file)
{
  DataLine data_line;
  data_line.set_delimiter(",");

  //
  // Skip the first two lines
  //
  ascii_file >> data_line;
  ascii_file >> data_line;

  //
  // The third line of the file contains the station name, latitude,
  // longitude, and elevation.
  //

  if (!(ascii_file >> data_line))
  {
    mlog << Error << "\nAeronetHandler::_readHeaderInfo() -> "
	 << "error reading station id line from input ASCII file \""
	 << ascii_file.filename() << "\"\n\n";

    return false;
  }

  //
  // Check for the correct number of columns in the header line
  //

  if (data_line.n_items() != NUM_HDR_COLS)
  {
    mlog << Error << "\nAeronetHandler::_readHeaderInfo() -> "
	 << "SURFRAD file has incorrect number of columns ("
	 << data_line.n_items() << ") in header line\n\n";
    return false;
  }

  //
  // Get the stationId
  //

  ConcatString stationId_string(data_line[0]);
  StringArray stationIdTokens = stationId_string.split("=");
  _stationId = stationIdTokens[stationIdTokens.n_elements()-1];

  //
  // Get the stationLon
  //

  ConcatString stationLon_string(data_line[1]);
  StringArray stationLonTokens = stationLon_string.split("=");
  _stationLon = atof(stationLonTokens[stationLonTokens.n_elements()-1]);

  //
  // Get the stationLat
  //

  ConcatString stationLat_string(data_line[2]);
  StringArray stationLatTokens = stationLat_string.split("=");
  _stationLat = atof(stationLatTokens[stationLatTokens.n_elements()-1]);

  //
  // Get the stationAlt
  //

  ConcatString stationAlt_string(data_line[3]);
  StringArray stationAltTokens = stationAlt_string.split("=");
  _stationAlt = atof(stationAltTokens[stationAltTokens.n_elements()-1]);

  return true;

}
