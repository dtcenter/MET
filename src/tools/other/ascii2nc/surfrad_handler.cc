// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "surfrad_handler.h"


const int SurfradHandler::NUM_HDR_COLS = 6;
const int SurfradHandler::NUM_OBS_COLS = 48;

const string SurfradHandler::HEADER_TYPE = "SRAD";

// NOTE: Convert values to standard units?

const int SurfradHandler::DW_PSP_GRIB_CODE = 204;
const int SurfradHandler::UW_PSP_GRIB_CODE = 211;
const int SurfradHandler::DIRECT_GRIB_CODE = 161;
const int SurfradHandler::DIFFUSE_GRIB_CODE = 167;
const int SurfradHandler::DW_PIR_GRIB_CODE = 205;
const int SurfradHandler::DW_CASETEMP_GRIB_CODE = -1;
const int SurfradHandler::DW_DOMETEMP_GRIB_CODE = -2;
const int SurfradHandler::UW_PIR_GRIB_CODE = 212;
const int SurfradHandler::UW_CASETEMP_GRIB_CODE = -3;
const int SurfradHandler::UW_DOMETEMP_GRIB_CODE = -4;
const int SurfradHandler::UVB_GRIB_CODE = -5;
const int SurfradHandler::PAR_GRIB_CODE = -6;
const int SurfradHandler::NETSOLAR_GRIB_CODE = 111;
const int SurfradHandler::NETIR_GRIB_CODE = 112;
const int SurfradHandler::TOTALNET_GRIB_CODE = 117;
const int SurfradHandler::TEMP_GRIB_CODE = 11;
const int SurfradHandler::RH_GRIB_CODE = 52;
const int SurfradHandler::WINDSPD_GRIB_CODE = 32;
const int SurfradHandler::WINDDIR_GRIB_CODE = 31;
const int SurfradHandler::PRESSURE_GRIB_CODE = 1;
const string DW_PSP_NAME       = "DW_PSP";
const string UW_PSP_NAME       = "UW_PSP";
const string DIRECT_NAME       = "DIRECT";
const string DIFFUSE_NAME      = "DIFFUSE";
const string DW_PIR_NAME       = "DW_PIR";
const string DW_CASETEMP_NAME  = "DW_CASETEMP";
const string DW_DOMETEMP_NAME  = "DW_DOMETEMP";
const string UW_PIR_NAME       = "UW_PIR_GRIB";
const string UW_CASETEMP_NAME  = "UW_CASETEMP";
const string UW_DOMETEMP_NAME  = "UW_DOMETEMP";
const string UVB_NAME          = "UVB";
const string PAR_NAME          = "PAR";
const string NETSOLAR_NAME     = "NETSOLAR";
const string NETIR_NAME        = "NETIR";
const string TOTALNET_NAME     = "TOTALNET";
const string TEMP_NAME         = "TMP";
const string RH_NAME           = "RH;";
const string WINDSPD_NAME      = "WSPD";
const string WINDDIR_NAME      = "WDIR";
const string PRESSURE_NAME     = "PRES";


  
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

bool SurfradHandler::_readObservations(LineDataFile &ascii_file)
{
  DataLine data_line;

  // Read and save the header information

  if (!_readHeaderInfo(ascii_file))
    return false;
   
   // Process the observation lines

   while (ascii_file >> data_line)
   {
     // Make sure that the line contains the correct number of tokens

     if (data_line.n_items() != NUM_OBS_COLS)
     {
       mlog << Error << "\nSurfradHandler::_readObservations() -> "
            << "line number " << data_line.line_number()
            << " does not have the correct number of columns  ("
            << NUM_OBS_COLS << ").\n\n";
       return false;
     }

     // Pull the valid time from the data line

     time_t valid_time = _getValidTime(data_line);
     if (valid_time == 0)
       return false;
     
     //
     // Save each of the observations from the line
     //

     double pressure = _pressMbToPa(atof(data_line[44]));
     
     // downwelling global solar (Watts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[9],
                                  DW_PSP_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[8]),
                                  DW_PSP_NAME));
     
     // upwelling global solar (Watts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[11],
                                  UW_PSP_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[10]),
                                  UW_PSP_NAME));
     
     // direct solar (Watts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[13],
                                  DIRECT_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[12]),
                                  DIRECT_NAME));
     
     // downwelling diffuse solar (Watts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[15],
                                  DIFFUSE_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[14]),
                                  DIFFUSE_NAME));
     
     // downwelling thermal infrared (Watts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[17],
                                  DW_PIR_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[16]),
                                  DW_PIR_NAME));
     
     // downwelling PIR case temp (K)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[19],
                                  DW_CASETEMP_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  _tempKtoC(atof(data_line[18])),
                                  DW_CASETEMP_NAME));
     
     // downwelling PIR dome temp (K)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[21],
                                  DW_DOMETEMP_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  _tempKtoC(atof(data_line[20])),
                                  DW_DOMETEMP_NAME));
     
     // upwelling thermal infrared (Watts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[23],
                                  UW_PIR_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[22]),
                                  UW_PIR_NAME));
     
     // upwelling PIR case temp (K)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[25],
                                  UW_CASETEMP_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  _tempKtoC(atof(data_line[24])),
                                  UW_CASETEMP_NAME));
     
     // upwelling PIR dome temp (K)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[27],
                                  UW_DOMETEMP_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  _tempKtoC(atof(data_line[26])),
                                  UW_DOMETEMP_NAME));
     
     // global UVB (milliWatts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[29],
                                  UVB_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[28]),
                                  UVB_NAME));
     
     // photosynthetically active radiation (Watts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[31],
                                  PAR_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[30]),
                                  PAR_NAME));
     
     // net solar (dw_psp - uw_psp) (Watts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[33],
                                  NETSOLAR_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[32]),
                                  NETSOLAR_NAME));
     
     // net infrared (dw_pir - uw_pir) (Watts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[35],
                                  NETIR_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[34]),
                                  NETIR_NAME));
     
     // net radiation (netsolar + netir) (Watts m^-2)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[37],
                                  TOTALNET_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[36]),
                                  TOTALNET_NAME));
     
     // 10-meter air temperature (C)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[39],
                                  TEMP_GRIB_CODE,
                                  bad_data_double, 10.0,
                                  atof(data_line[38]),
                                  TEMP_NAME));
     
     // relative humidity (%)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[41],
                                  RH_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[40]),
                                  RH_NAME));
     
     // wind speed (ms^-1)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[43],
                                  WINDSPD_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[42]),
                                  WINDSPD_NAME));
     
     // wind direction (degrees, clockwise from north)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[45],
                                  WINDDIR_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  atof(data_line[44]),
                                  WINDDIR_NAME));
     
     // station pressure (mb)

     _addObservations(Observation(HEADER_TYPE, _stationId,
                                  valid_time,
                                  _stationLat, _stationLon,
                                  _stationAlt,
                                  data_line[47],
                                  PRESSURE_GRIB_CODE,
                                  bad_data_double, 0.0,
                                  pressure,
                                  PRESSURE_NAME));
   } // end while

   return true;
}
  
////////////////////////////////////////////////////////////////////////

time_t SurfradHandler::_getValidTime(const DataLine &data_line) const
{
  struct tm time_struct;
  memset(&time_struct, 0, sizeof(time_struct));
  
  time_struct.tm_year = atoi(data_line[0]) - 1900;
  time_struct.tm_mon = atoi(data_line[2]) - 1;
  time_struct.tm_mday = atoi(data_line[3]);
  time_struct.tm_hour = atoi(data_line[4]);
  time_struct.tm_min = atoi(data_line[5]);
  
  return timegm(&time_struct);
}

////////////////////////////////////////////////////////////////////////

bool SurfradHandler::_readHeaderInfo(LineDataFile &ascii_file)
{
  DataLine data_line;
  
  // The first line of the file contains the station name.  We need to
  // put all of the tokens back together to recreate the station name.
  // There may be an easier way to do this, but I didn't see anything
  // in the DataLine class.

  if (!(ascii_file >> data_line))
  {
    mlog << Error << "\nSurfradHandler::_readHeaderInfo() -> "
         << "error reading station id line from input ASCII file \""
         << ascii_file.filename() << "\"\n\n";

    return false;
  }

  //
  // Replace spaces in the station name with underscores to prevent
  // embedded whitespace.
  //

  _stationId = data_line[0];
  for (int i = 1; i < data_line.n_items(); ++i)
    _stationId += string("_") + data_line[i];

  // The second line of the file has the lat/lon/alt information

  if (!(ascii_file >> data_line))
  {
    mlog << Error << "\nSurfradHandler::_readHeaderInfo() -> "
         << "error reading location line from input ASCII file \""
         << ascii_file.filename() << "\"\n\n";

    return false;
  }
  
  if (data_line.n_items() != NUM_HDR_COLS)
  {
    mlog << Error << "\nSurfradHandler::_readHeaderInfo() -> "
         << "SURFRAD file has incorrect number of columns ("
         << data_line.n_items() << ") in header line\n\n";
    return false;
  }
  
  _stationLat = atof(data_line[0]);
  _stationLon = atof(data_line[1]);
  _stationAlt = atof(data_line[2]);
  
  return true;
}

