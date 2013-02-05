

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
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

#include "surfrad_handler.h"


const int SurfradHandler::NUM_HDR_COLS = 6;
const int SurfradHandler::NUM_OBS_COLS = 48;

// NOTE: Convert values to standard units?

const int SurfradHandler::DW_PSP_GRIB_CODE = 204;
const int SurfradHandler::UW_PSP_GRIB_CODE = 211;
const int SurfradHandler::DIRECT_GRIB_CODE = 161;
const int SurfradHandler::DIFFUSE_GRIB_CODE = 167;
const int SurfradHandler::DW_PIR_GRIB_CODE = 205;
const int SurfradHandler::DW_CASETEMP_GRIB_CODE = 0;
const int SurfradHandler::DW_DOMETEMP_GRIB_CODE = 0;
const int SurfradHandler::UW_PIR_GRIB_CODE = 212;
const int SurfradHandler::UW_CASETEMP_GRIB_CODE = 0;
const int SurfradHandler::UW_DOMETEMP_GRIB_CODE = 0;
const int SurfradHandler::UVB_GRIB_CODE = 0;
const int SurfradHandler::PAR_GRIB_CODE = 0;
const int SurfradHandler::NETSOLAR_GRIB_CODE = 111;
const int SurfradHandler::NETIR_GRIB_CODE = 112;
const int SurfradHandler::TOTALNET_GRIB_CODE = 117;
const int SurfradHandler::TEMP_GRIB_CODE = 11;
const int SurfradHandler::RH_GRIB_CODE = 52;
const int SurfradHandler::WINDSPD_GRIB_CODE = 32;
const int SurfradHandler::WINDDIR_GRIB_CODE = 31;
const int SurfradHandler::PRESSURE_GRIB_CODE = 1;

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SurfradHandler
   //


////////////////////////////////////////////////////////////////////////


SurfradHandler::SurfradHandler(const string &program_name) :
  FileHandler(program_name)
{
}


////////////////////////////////////////////////////////////////////////


SurfradHandler::~SurfradHandler()
{
}


////////////////////////////////////////////////////////////////////////


bool SurfradHandler::isFileType(LineDataFile &ascii_file) const
{
  //
  // Initialize the return value.
  //
  bool is_file_type = false;
  
  //
  // Read the first line from the file.  We will skip this line since it
  // contains the station name which can contain spaces so we don't know
  // how many tokens it will have.
  //
  DataLine dl;
  ascii_file >> dl;

  //
  // Read the second line from the file.  This line looks something like
  // the following:
  //    40.125 -105.237 1689 m version 1
  //
  ascii_file >> dl;

  //
  // Check for expected number of columns
  //
  if (dl.n_items() == NUM_HDR_COLS)
    is_file_type = true;
   
  //
  // Read the third line from the file.  This is the first observation line.
  //
  ascii_file >> dl;

  //
  // Check for expected number of columns
  //
  
  if (dl.n_items() == NUM_OBS_COLS)
    is_file_type = true;
   
  return is_file_type;
}
  

////////////////////////////////////////////////////////////////////////
// Private/Protected methods
////////////////////////////////////////////////////////////////////////

int SurfradHandler::_getLocation(const DataLine &data_line)
{
  // Make sure the line has the correct number of tokens

  if (data_line.n_items() != NUM_HDR_COLS)
  {
    mlog << Error << "\nSurfradHandler::_getLocation() -> "
	 << "SURFRAD file has incorrect number of columns ("
	 << data_line.n_items() << ") in header line\n\n";
    return false;
  }
  
  // Set the location values

  _stationLat = atof(data_line[0]);
  _stationLon = atof(data_line[1]);
  _stationAlt = atof(data_line[2]);
  
  return true;
}

////////////////////////////////////////////////////////////////////////

bool SurfradHandler::_prepareHeaders(LineDataFile &ascii_file)
{
  // The SURFRAD files contain a separate record for each data time.
  // Because of this, we can just count the number of lines in the file
  // to figure out how many header records there will be.  Of course, we
  // have to remember to subtract out the header lines in the file.

  // Count the number of lines in the file

  DataLine dl;
  int num_lines = 0;
  while (ascii_file >> dl)
    ++num_lines;
  
  // Set the number of header records global variable

  _nhdr += num_lines - 2;

  return true;
}

////////////////////////////////////////////////////////////////////////

int SurfradHandler::_getStationId(const DataLine &data_line)
{
  _stationId = data_line[0];
  
  for (int i = 1; i < data_line.n_items(); ++i)
    _stationId += string(" ") + data_line[i];
  
  return true;
}

////////////////////////////////////////////////////////////////////////

string SurfradHandler::_getValidTime(const DataLine &data_line) const
{
  int year = atoi(data_line[0]);
  int month = atoi(data_line[2]);
  int day = atoi(data_line[3]);
  int hour = atoi(data_line[4]);
  int minute = atoi(data_line[5]);
  
  char time_string[80];
  
  sprintf(time_string, "%04d%02d%02d_%02d%02d00",
	  year, month, day, hour, minute);
  
  return string(time_string);
}

////////////////////////////////////////////////////////////////////////

bool SurfradHandler::_processObs(LineDataFile &ascii_file,
				 const string &nc_filename) {
   DataLine data_line;

   mlog << Debug(2) << "Processing observations for " << _nhdr
        << " headers.\n";

   // The first line of the file contains the station name.

   ascii_file >> data_line;
   if (!_getStationId(data_line))
     return false;
   
   // The second line of the file has the lat/lon/alt information

   ascii_file >> data_line;
   if (!_getLocation(data_line))
     return false;
   
   while (ascii_file >> data_line)
   {
     // Make sure that the line contains the correct number of tokens

     if (data_line.n_items() != NUM_OBS_COLS)
     {
       mlog << Error << "\nSurfradHandler::_processObs() -> "
	    << "line number " << data_line.line_number()
	    << " does not have the correct number of columns  ("
	    << NUM_OBS_COLS << ").\n\n";
       return false;
     }

     // Pull the valid time from the data line

     string valid_time = _getValidTime(data_line);
     if (valid_time == "")
       return false;
     
     // Write the header info
     
     if (!_writeHdrInfo("ADPSFC", _stationId.c_str(),
			valid_time.c_str(),
			_stationLat, _stationLon, _stationAlt))
       return false;
   
     // Write each of the observations from the line

     // downwelling global solar (Watts m^-2)

     _writeObsInfo(DW_PSP_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[8]), data_line[9]);
     
     // upwelling global solar (Wats m^-2)

     _writeObsInfo(UW_PSP_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[10]), data_line[11]);
     
     // direct solar (Watts m^-2)

     _writeObsInfo(DIRECT_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[12]), data_line[13]);
     
     // downwelling diffuse solar (Watts m^-2)

     _writeObsInfo(DIFFUSE_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[14]), data_line[15]);
     
     // downwelling thermal infrared (Watts m^-2)

     _writeObsInfo(DW_PIR_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[16]), data_line[17]);
     
     // downwelling PIR case temp (K)

     _writeObsInfo(DW_CASETEMP_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   _tempKtoC(atof(data_line[18])), data_line[19]);
     
     // downwelling PIR dome temp (K)

     _writeObsInfo(DW_DOMETEMP_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   _tempKtoC(atof(data_line[20])), data_line[21]);
     
     // upwelling thermal infrared (Watts m^-2)

     _writeObsInfo(UW_PIR_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[22]), data_line[23]);
     
     // upwelling PIR case temp (K)

     _writeObsInfo(UW_CASETEMP_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   _tempKtoC(atof(data_line[24])), data_line[25]);
     
     // upwelling PIR dome temp (K)

     _writeObsInfo(UW_DOMETEMP_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   _tempKtoC(atof(data_line[26])), data_line[27]);
     
     // global UVB (milliWatts m^-2)

     _writeObsInfo(UVB_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[28]), data_line[29]);
     
     // photosynthetically active radiation (Watts m^-2)

     _writeObsInfo(PAR_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[30]), data_line[31]);
     
     // net solar (dw_psp - uw_psp) (Watts m^-2)

     _writeObsInfo(NETSOLAR_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[32]), data_line[33]);
     
     // net infrared (dw_pir - uw_pir) (Watts m^-2)

     _writeObsInfo(NETIR_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[34]), data_line[35]);
     
     // net radiation (netsolar + netir) (Watts m^-2)

     _writeObsInfo(TOTALNET_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[36]), data_line[37]);
     
     // 10-meter air temperature (C)

     _writeObsInfo(TEMP_GRIB_CODE,
		   FILL_VALUE, 10.0,
		   atof(data_line[38]), data_line[39]);
     
     // relative humidity (%)

     _writeObsInfo(RH_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[40]), data_line[41]);
     
     // wind speed (ms^-1)

     _writeObsInfo(WINDSPD_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[42]), data_line[43]);
     
     // wind direction (degrees, clockwise from north)

     _writeObsInfo(WINDDIR_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   atof(data_line[44]), data_line[45]);
     
     // station pressure (mb)

     _writeObsInfo(PRESSURE_GRIB_CODE,
		   FILL_VALUE, 0.0,
		   _pressMbToPa(atof(data_line[46])), data_line[47]);
     
   } // end while

   return true;
}

