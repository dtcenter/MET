

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
   // ** University Corporation for Atmospheric Research (UCAR)
   // ** National Center for Atmospheric Research (NCAR)
   // ** Research Applications Lab (RAL)
   // ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


using namespace std;

#include <algorithm>
#include <iostream>

#include "vx_nc_util.h"

#include "file_handler.h"


const int FileHandler::HDR_ARRAY_LEN  = 3;  // Observation header length
const int FileHandler::OBS_ARRAY_LEN  = 5;  // Observation values length

const float FileHandler::FILL_VALUE = -9999.f;

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class FileHandler
   //


////////////////////////////////////////////////////////////////////////


FileHandler::FileHandler(const string &program_name) :
  _programName(program_name),
  _ncFile(0),
  _hdrTypeVar(0),
  _hdrStationIdVar(0),
  _hdrValidTimeVar(0),
  _hdrArrayVar(0),
  _obsQualityVar(0),
  _obsArrayVar(0),
  _nhdr(0)
{
}

////////////////////////////////////////////////////////////////////////

FileHandler::~FileHandler()
{
  delete _ncFile;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::readAsciiFiles(const vector< ConcatString > &ascii_filename_list)
{
  // Loop through the ASCII files, reading in the observations.  At the end of
  // this loop, all of the observations will be in the _observations vector.

  for (vector< ConcatString >::const_iterator ascii_filename =
	 ascii_filename_list.begin();
       ascii_filename != ascii_filename_list.end(); ++ascii_filename)
  {
    // Open the input ASCII observation file

    LineDataFile ascii_file;

    if (!ascii_file.open(*ascii_filename))
    {
      mlog << Error << "\nFileHandler::processFiles() -> "
	   << "can't open input ASCII file \"" << *ascii_filename
	   << "\" for reading\n\n";

      return false;
    }
    
    // Read the observations

    if (!_readObservations(ascii_file))
      return false;

    // Close the file

    ascii_file.close();
  }

  return true;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::writeNetcdfFile(const string &nc_filename)
{
  // Loop through the observations, counting the number of headers needed in
  // the netCDF file.  We need to count the headers before opening the netCDF
  // file because we can't have two "unlimited" dimensions in a netCDF file.

  _countHeaders();
  
  mlog << Debug(2) << "Processing observations for " << _nhdr
       << " headers.\n";

  // Open the netCDF file.  This can't be done until after we process the
  // headers and set _nhdrs to the number of header records.

  if (!_openNetcdf(nc_filename))
    return false;
  
  // Write the headers and observations to the netCDF file.

  if (!_writeObservations())
    return false;
  
  // Close the netCDF file.

  _closeNetcdf();
  
  mlog << Debug(2) << "Finished processing " << _obsNum + 1
       << " observations for " << _hdrNum + 1 << " headers.\n";

  return true;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::summarizeObs(const time_t start_time, const time_t end_time,
			       const int interval_secs)
{
  // Sort the observations.  This will put them in chronological order, with
  // secondary sorts on things like the station id

  sort(_observations.begin(), _observations.end());
  
  // Skip any observations before our start time

  vector< Observation >::const_iterator curr_obs = _observations.begin();
  
  while (curr_obs != _observations.end() &&
	 curr_obs->getValidTime() < start_time)
    ++curr_obs;
  
  // Loop through the time periods, summarizing as we go

  map< SummaryKey, NumArray* > summary_values;
  vector< Observation > summary_obs;
  
  for (time_t curr_start_time = start_time; curr_start_time < end_time;
       curr_start_time += interval_secs)
  {
    time_t curr_end_time = curr_start_time + interval_secs - 1;
    
    // Add all observations that are within the current time period to the
    // summary values

    while (curr_obs != _observations.end() &&
	   curr_obs->getValidTime() <= curr_end_time)
    {
      SummaryKey summary_key(curr_obs->getHeaderType(),
			     curr_obs->getStationId(),
			     curr_obs->getLatitude(),
			     curr_obs->getLongitude(),
			     curr_obs->getElevation(),
			     curr_obs->getGribCode());
      
      // If this is a new station/grib code pair, create a new NumArray

      if (summary_values.find(summary_key) == summary_values.end())
	summary_values[summary_key] = new NumArray;
      
      summary_values[summary_key]->add(curr_obs->getValue());
      
      ++curr_obs;
    }
    
    // If we get here, we've found all of the observations for the current
    // time period so we need to calculate the summaries.

    map< SummaryKey, NumArray* >::const_iterator curr_values;
    for (curr_values = summary_values.begin();
	 curr_values != summary_values.end(); ++curr_values)
    {
      // mean

      double mean, sdev;
      curr_values->second->compute_mean_stdev(mean, sdev);
      
      summary_obs.push_back(Observation(curr_values->first.getHeaderType() + "_MEAN",
					curr_values->first.getStationId(),
					curr_start_time, curr_end_time,
					curr_values->first.getLatitude(),
					curr_values->first.getLongitude(),
					curr_values->first.getElevation(),
					"",
					curr_values->first.getGribCode(),
					interval_secs, FILL_VALUE,
					mean));
	
      // sdev

      summary_obs.push_back(Observation(curr_values->first.getHeaderType() + "_SDEV",
					curr_values->first.getStationId(),
					curr_start_time, curr_end_time,
					curr_values->first.getLatitude(),
					curr_values->first.getLongitude(),
					curr_values->first.getElevation(),
					"",
					curr_values->first.getGribCode(),
					interval_secs, FILL_VALUE,
					sdev));
	
      // min

      summary_obs.push_back(Observation(curr_values->first.getHeaderType() + "_MIN",
					curr_values->first.getStationId(),
					curr_start_time, curr_end_time,
					curr_values->first.getLatitude(),
					curr_values->first.getLongitude(),
					curr_values->first.getElevation(),
					"",
					curr_values->first.getGribCode(),
					interval_secs, FILL_VALUE,
					curr_values->second->min()));
	
      // max

      summary_obs.push_back(Observation(curr_values->first.getHeaderType() + "_MAX",
					curr_values->first.getStationId(),
					curr_start_time, curr_end_time,
					curr_values->first.getLatitude(),
					curr_values->first.getLongitude(),
					curr_values->first.getElevation(),
					"",
					curr_values->first.getGribCode(),
					interval_secs, FILL_VALUE,
					curr_values->second->max()));
	
      // median

      summary_obs.push_back(Observation(curr_values->first.getHeaderType() + "_MEDIAN",
					curr_values->first.getStationId(),
					curr_start_time, curr_end_time,
					curr_values->first.getLatitude(),
					curr_values->first.getLongitude(),
					curr_values->first.getElevation(),
					"",
					curr_values->first.getGribCode(),
					interval_secs, FILL_VALUE,
					curr_values->second->percentile_array(0.5)));
	
    } /* endfor - curr_values */
      
    // Clear out the value arrays to get ready for the next time period

    map< SummaryKey, NumArray* >::iterator summary_iter;
    for (summary_iter = summary_values.begin();
	 summary_iter != summary_values.end(); ++summary_iter)
      delete summary_iter->second;
    
    summary_values.clear();
  }
  
  // Replace the observations vector with the summary observations

  _observations = summary_obs;
  
  return true;
}


////////////////////////////////////////////////////////////////////////
// Protected/Private Methods
////////////////////////////////////////////////////////////////////////

void FileHandler::_closeNetcdf()
{
   _ncFile->close();
   delete _ncFile;
   _ncFile = (NcFile *) 0;
}

////////////////////////////////////////////////////////////////////////

void FileHandler::_countHeaders()
{
  _nhdr = 0;
  
  string prev_header_type = "";
  string prev_station_id = "";
  time_t prev_valid_time = 0;
  
  for (vector< Observation >::const_iterator obs = _observations.begin();
       obs != _observations.end(); ++obs)
  {
    if (obs->getHeaderType() != prev_header_type ||
	obs->getStationId() != prev_station_id ||
	obs->getValidTime() != prev_valid_time)
    {
      _nhdr++;
      
      prev_header_type = obs->getHeaderType();
      prev_station_id = obs->getStationId();
      prev_valid_time = obs->getValidTime();
    }
    
  } /* endfor - obs */
  
}

////////////////////////////////////////////////////////////////////////

time_t FileHandler::_getValidTime(const string &time_string) const
{
  struct tm time_struct;
  memset(&time_struct, 0, sizeof(time_struct));
  
  time_struct.tm_year = atoi(time_string.substr(0, 4).c_str()) - 1900;
  time_struct.tm_mon = atoi(time_string.substr(4, 2).c_str()) - 1;
  time_struct.tm_mday = atoi(time_string.substr(6, 2).c_str());
  time_struct.tm_hour = atoi(time_string.substr(9, 2).c_str());
  time_struct.tm_min = atoi(time_string.substr(11, 2).c_str());
  time_struct.tm_sec = atoi(time_string.substr(13, 2).c_str());
  
  return timegm(&time_struct);
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::_openNetcdf(const string &nc_filename)
{
   mlog << Debug(1) << "Creating NetCDF Observation file: "
        << nc_filename << "\n";

   //
   // Create the output NetCDF file for writing
   //
   _ncFile = new NcFile(nc_filename.c_str(), NcFile::Replace);
   
   if(!_ncFile->is_valid()) {
      mlog << Error << "\nFileHandler::_openNetcdf() -> "
           << "can't open output NetCDF file \"" << nc_filename
           << "\" for writing\n\n";
      _closeNetcdf();

      return false;
   }

   //
   // Define the NetCDF dimensions
   //
   NcDim *strl_dim    = _ncFile->add_dim("mxstr",
					 (string("YYYYMMDD_HHMMSS").size() * 2) + 2);
   NcDim *hdr_arr_dim = _ncFile->add_dim("hdr_arr_len", (long) HDR_ARRAY_LEN);
   NcDim *obs_arr_dim = _ncFile->add_dim("obs_arr_len", (long) OBS_ARRAY_LEN);
   NcDim *hdr_dim     = _ncFile->add_dim("nhdr", (long)_nhdr);
   NcDim *obs_dim     = _ncFile->add_dim("nobs"); // unlimited dimension

   //
   // Add variables to NetCDF file
   //
   _hdrTypeVar = _ncFile->add_var("hdr_typ", ncChar, hdr_dim, strl_dim);
   _hdrStationIdVar = _ncFile->add_var("hdr_sid", ncChar, hdr_dim, strl_dim);
   _hdrValidTimeVar = _ncFile->add_var("hdr_vld", ncChar, hdr_dim, strl_dim);
   _hdrArrayVar = _ncFile->add_var("hdr_arr", ncFloat, hdr_dim, hdr_arr_dim);
   _obsQualityVar = _ncFile->add_var("obs_qty", ncChar, obs_dim, strl_dim);
   _obsArrayVar = _ncFile->add_var("obs_arr", ncFloat, obs_dim, obs_arr_dim);

   //
   // Add attributes to the NetCDF variables
   //
   _hdrTypeVar->add_att("long_name", "message type");
   _hdrStationIdVar->add_att("long_name", "station identification");
   _hdrValidTimeVar->add_att("long_name", "valid time");
   _hdrValidTimeVar->add_att("units", "YYYYMMDD_HHMMSS UTC");

   _hdrArrayVar->add_att("long_name", "array of observation station header values");
   _hdrArrayVar->add_att("_fill_value", FILL_VALUE);
   _hdrArrayVar->add_att("columns", "lat lon elv");
   _hdrArrayVar->add_att("lat_long_name", "latitude");
   _hdrArrayVar->add_att("lat_units", "degrees_north");
   _hdrArrayVar->add_att("lon_long_name", "longitude");
   _hdrArrayVar->add_att("lon_units", "degrees_east");
   _hdrArrayVar->add_att("elv_long_name", "elevation ");
   _hdrArrayVar->add_att("elv_units", "meters above sea level (msl)");

   _obsQualityVar->add_att("long_name", "quality flag");

   _obsArrayVar->add_att("long_name", "array of observation values");
   _obsArrayVar->add_att("_fill_value", FILL_VALUE);
   _obsArrayVar->add_att("columns", "hdr_id gc lvl hgt ob");
   _obsArrayVar->add_att("hdr_id_long_name", "index of matching header data");
   _obsArrayVar->add_att("gc_long_name", "grib code corresponding to the observation type");
   _obsArrayVar->add_att("lvl_long_name", "pressure level (hPa) or accumulation interval (sec)");
   _obsArrayVar->add_att("hgt_long_name", "height in meters above sea level or ground level (msl or agl)");
   _obsArrayVar->add_att("ob_long_name", "observation value");

   //
   // Add global attributes
   //
   write_netcdf_global(_ncFile, nc_filename.c_str(), _programName.c_str());

   //
   // Initialize the header and observation record counters
   //
   _hdrNum = -1;
   _obsNum = -1;
   
   return true;
}


////////////////////////////////////////////////////////////////////////

bool FileHandler::_writeHdrInfo(const ConcatString &hdr_typ,
				const ConcatString &hdr_sid,
				const ConcatString &hdr_vld,
				double lat, double lon, double elv) {
  float hdr_arr[HDR_ARRAY_LEN];

   //
   // Increment header count before writing
   //
   _hdrNum++;
   
   //
   // Build the header array
   //
   hdr_arr[0] = lat;
   hdr_arr[1] = lon;
   hdr_arr[2] = elv;
   
   //
   // Store the message type
   //
   if(!_hdrTypeVar->set_cur(_hdrNum, (long) 0) ||
      !_hdrTypeVar->put(hdr_typ, (long) 1, (long) hdr_typ.length())) {
      mlog << Error << "\nFileHandler::_writeHdrInfo() -> "
           << "error writing the message type to the NetCDF file\n\n";
      return false;
   }

   //
   // Store the station id
   //
   if(!_hdrStationIdVar->set_cur(_hdrNum, (long) 0) ||
      !_hdrStationIdVar->put(hdr_sid, (long) 1, (long) hdr_sid.length())) {
      mlog << Error << "\nFileHandler::_writeHdrInfo() -> "
           << "error writing the station id to the NetCDF file\n\n";
      return false;
   }

   //
   // Store the valid time and check that it's is in the expected
   // time format: YYYYMMDD_HHMMSS
   //
   if(check_reg_exp(yyyymmdd_hhmmss_reg_exp, hdr_vld) != true) {
      mlog << Error << "\nFileHandler::_writeHdrInfo() -> "
           << "valid time is not in the expected YYYYMMDD_HHMMSS format: "
           << hdr_vld << "\n\n";
      return false;
   }

   if(!_hdrValidTimeVar->set_cur(_hdrNum, (long) 0) ||
      !_hdrValidTimeVar->put(hdr_vld, (long) 1, (long) hdr_vld.length())) {
      mlog << Error << "\nFileHandler::_nwriteHdrInfo() -> "
           << "error writing the valid time to the NetCDF file\n\n";
      return false;
   }

   //
   // Store the header array
   //
   if(!_hdrArrayVar->set_cur(_hdrNum, (long) 0) ||
      !_hdrArrayVar->put(hdr_arr, (long) 1, (long) HDR_ARRAY_LEN) ) {
      mlog << Error << "\nFileHandler::_writeHdrInfo() -> "
           << "error writing the header array to the NetCDF file\n\n";
      return false;
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::_writeObsInfo(int gc, float prs, float hgt, float obs,
				const ConcatString &qty) {
   float obs_arr[OBS_ARRAY_LEN];
   
   //
   // Increment observation count before writing
   //
   _obsNum++;
   
   //
   // Build the observation array
   //
   obs_arr[0] = _hdrNum; // Index of header
   obs_arr[1] = gc;    // GRIB code corresponding to the observation type
   obs_arr[2] = prs;   // Pressure level (hPa) or accumulation interval (sec)
   obs_arr[3] = hgt;   // Height in meters above sea level or ground level (msl or agl)
   obs_arr[4] = obs;   // Observation value

   //
   // Write the observation array
   //
   if(!_obsArrayVar->set_cur(_obsNum, (long) 0) ||
      !_obsArrayVar->put(obs_arr, (long) 1, (long) OBS_ARRAY_LEN) ) {
      mlog << Error << "\nFileHandler::_writeObsInfo() -> "
           << "error writing the observation array to the NetCDF file\n\n";
      return false;
   }

   //
   // Write the observation flag value
   //
   if(!_obsQualityVar->set_cur(_obsNum, (long) 0) ||
      !_obsQualityVar->put(qty, (long) 1, (long) qty.length()) ) {
      mlog << Error << "\nFileHandler::_writeObsInfo() -> "
           << "error writing the quality flag to the NetCDF file\n\n";
      return false;
   }
   
   return true;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::_writeObservations()
{
  string prev_header_type = "";
  string prev_station_id = "";
  time_t prev_valid_time = 0;
  
  for (vector< Observation >::const_iterator obs = _observations.begin();
       obs != _observations.end(); ++obs)
  {
    if (obs->getHeaderType() != prev_header_type ||
	obs->getStationId() != prev_station_id ||
	obs->getValidTime() != prev_valid_time)
    {
      if (!_writeHdrInfo(obs->getHeaderType().c_str(),
			 obs->getStationId().c_str(),
			 obs->getValidTimeString().c_str(),
			 obs->getLatitude(),
			 obs->getLongitude(),
			 obs->getElevation()))
	return false;
      
      prev_header_type = obs->getHeaderType();
      prev_station_id = obs->getStationId();
      prev_valid_time = obs->getValidTime();
    }
    
    if (!_writeObsInfo(obs->getGribCode(),
		       obs->getPressureLevel(),
		       obs->getHeight(),
		       obs->getValue(),
		       obs->getQualityFlag().c_str()))
      return false;
    
  } /* endfor - obs */
  
  return true;
}
  
