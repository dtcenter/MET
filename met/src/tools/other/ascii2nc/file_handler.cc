// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <algorithm>
#include <iostream>

#include "vx_math.h"
#include "vx_nc_util.h"

#include "file_handler.h"

#include "summary_calc_max.h"
#include "summary_calc_mean.h"
#include "summary_calc_median.h"
#include "summary_calc_min.h"
#include "summary_calc_percentile.h"
#include "summary_calc_range.h"
#include "summary_calc_stdev.h"


const long FileHandler::HDR_ARRAY_LEN  = 3;  // Observation header length
const long FileHandler::OBS_ARRAY_LEN  = 5;  // Observation values length
const long FileHandler::MAX_STRING_LEN = 40; // Maximum length for strings
                                             //   in the netCDF file

const float FileHandler::FILL_VALUE = -9999.f;

const int DEF_DEFALTE_LEVEL = 2;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class FileHandler
   //


////////////////////////////////////////////////////////////////////////


FileHandler::FileHandler(const string &program_name) :
  _programName(program_name),
  _ncFile(0),
  _nhdr(0),
  _hdrNum(0),
  _obsNum(0),
  _gridMaskNum(0),
  _polyMaskNum(0),
  _sidMaskNum(0),
  _gridMask(0),
  _polyMask(0),
  _sidMask(0),
  use_var_id(false),
  deflate_level(DEF_DEFLATE_LEVEL),
  _dataSummarized(false)
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

  for (vector< ConcatString >::const_iterator ascii_filename = ascii_filename_list.begin();
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

  // List the number of rejected observations.

  mlog << Debug(2)
       << "Rejected " << _gridMaskNum
       << " observations off the masking grid.\n"
       << "Rejected " << _polyMaskNum
       << " observations outside the masking polyline.\n"
       << "Rejected " << _sidMaskNum
       << " observations not matched with station ID's.\n";

  // Loop through the observations, counting the number of headers needed in
  // the netCDF file.  We need to count the headers before opening the netCDF
  // file because we can't have two "unlimited" dimensions in a netCDF file.

  _countHeaders();

  // Check for no data
  if (_nhdr == 0)
  {
    mlog << Error << "\nZero observations retained.\n"
         << "Cannot create NetCDF Observation file: "
         << nc_filename << "\n\n";

    return false;
  }

  mlog << Debug(2) << "Processing observations for " << _nhdr
       << " headers.\n";

  // Open the netCDF file.  This can't be done until after we process the
  // headers and set _nhdrs to the number of header records.

  if (!_openNetcdf(nc_filename))
    return false;

  // If we were summarizing, add global attributes showing how the
  // summarization was done.

  if (_dataSummarized)
  {
    add_att(_ncFile, "time_summary_beg",
                     _secsToTimeString(_summaryInfo.beg));
    add_att(_ncFile, "time_summary_end",
                     _secsToTimeString(_summaryInfo.end));

    char att_string[1024];

    sprintf(att_string, "%d", _summaryInfo.step);
    add_att(_ncFile, "time_summary_step", att_string);

    sprintf(att_string, "%d", _summaryInfo.width);
    add_att(_ncFile, "time_summary_width", att_string);

    string grib_code_string;
    for (int i = 0; i < _summaryInfo.grib_code.n_elements(); ++i)
    {
      sprintf(att_string, "%d", _summaryInfo.grib_code[i]);
      if (i == 0)
        grib_code_string = string(att_string);
      else
        grib_code_string += string(" ") + att_string;
    }
    add_att(_ncFile, "time_summary_grib_code", grib_code_string.c_str());

    string type_string;
    for (int i = 0; i < _summaryInfo.type.n_elements(); ++i)
    {
      if (i == 0)
        type_string = _summaryInfo.type[i];
      else
        type_string += string(" ") + _summaryInfo.type[i];
    }
    add_att(_ncFile, "time_summary_type", type_string.c_str());
  }

  // Write the headers and observations to the netCDF file.

  if (!_writeObservations())
    return false;

  // Add variable names
  if (use_var_id) {
    int max_name_len = _MAX_STRING_LEN;
    char var_name[max_name_len];
    add_att(_ncFile, nc_att_use_var_id, "true");
    NcDim var_dim  = add_dim(_ncFile, nc_dim_nvar, (long)obs_names.n_elements());
    NcDim name_dim = add_dim(_ncFile, nc_dim_name, (long)max_name_len);
    NcVar var_var_name  = add_var(_ncFile, nc_var_name, ncChar, var_dim, name_dim, deflate_level);
    add_att(&var_var_name,  "long_name", "variable names from ASCII input");
    
    long offsets[2] = { 0, 0 };
    long lengths[2] = { 1, max_name_len } ;
    for(int i=0; i<obs_names.n_elements(); i++) {
      for(int tIdx=0; tIdx<max_name_len; tIdx++) {
        var_name[tIdx] = bad_data_char;
      }
      strcpy(var_name, obs_names[i]);

      if(!put_nc_data(&var_var_name, (char *)var_name, lengths, offsets)) {
         mlog << Error << "\nwriteNetcdfFile() -> "
              << "error writing the variable name to the netCDF file\n\n";
         exit(1);
      }
      offsets[0] += 1;
    } // end for i
  }
  
  // Close the netCDF file.

  _closeNetcdf();

  mlog << Debug(2) << "Finished processing " << _obsNum + 1
       << " observations for " << _hdrNum + 1 << " headers.\n";

  return true;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::summarizeObs(const TimeSummaryInfo &summary_info)
{
  // Save the summary information
  const char *var_name = 0;

  _dataSummarized = true;
  _summaryInfo = summary_info;

  // Initialize the list of summary observations

  StringArray summary_vnames;
  vector< Observation > summary_obs;

  // Sort the observations.  This will put them in chronological order, with
  // secondary sorts of things like the station id

  sort(_observations.begin(), _observations.end());

  // Check for zero observations

  if (_observations.size() == 0) return true;

  // Extract the desired time intervals from the summary information.
  // The vector will be in chronological order by start time, but could
  // overlap in time.

  vector< TimeSummaryInterval > time_intervals =
    _getTimeIntervals(_observations[0].getValidTime(),
                      _observations[_observations.size()-1].getValidTime(),
                      summary_info);

  // Get the summary calculators from the summary information.

  vector< SummaryCalc* > calculators = _getSummaryCalculators(summary_info);

  // Get a pointer into the observations

  vector< Observation >::const_iterator curr_obs = _observations.begin();

  // Loop through the time periods, processing the appropriate observations

  vector< TimeSummaryInterval >::const_iterator time_interval;

  for (time_interval = time_intervals.begin();
       time_interval != time_intervals.end(); ++time_interval)
  {
    // Initialize the map used to sort observations in this time period
    // into their correct summary groups

    map< SummaryKey, NumArray* > summary_values;

    // Loop backwards through the observations to find the first observation
    // in the interval.  We need to do this because the user can define
    // overlapping intervals.

    while (curr_obs != _observations.begin() &&
           curr_obs->getValidTime() > time_interval->getStartTime())
      --curr_obs;

    // At this point, we are either at the beginning of the observations list
    // or we are at the observation right before our current interval.  Process
    // observations until we get to the end of the interval.

    while (curr_obs != _observations.end() &&
           curr_obs->getValidTime() < time_interval->getEndTime())
    {
      // We need to double-check that this observation is indeed within the
      // current time interval.  This takes care of the cases where there is
      // space between the time intervals and when we are first starting out.
      // It also allows us to go back one observation too far when looking for
      // the first observation in this time interval.

      if (time_interval->isInInterval(curr_obs->getValidTime()) &&
          summary_info.grib_code.has(curr_obs->getGribCode()))
      {
      // The summary key defines which observations should be grouped
      // together.  Any differences in key values indicates a different
      // summary.

        SummaryKey summary_key(curr_obs->getHeaderType(),
                               curr_obs->getStationId(),
                               curr_obs->getLatitude(),
                               curr_obs->getLongitude(),
                               curr_obs->getElevation(),
                               curr_obs->getGribCode(),
                               curr_obs->getHeight(),
                               curr_obs->getPressureLevel(),
                               curr_obs->getVarName());

        // Collect variable names
        var_name = curr_obs->getVarName().c_str();
        if (0 < strlen(var_name) && !summary_vnames.has(var_name)) {
          summary_vnames.add(var_name);
        }
      // If this is a new key, create a new NumArray

        if (summary_values.find(summary_key) == summary_values.end())
          summary_values[summary_key] = new NumArray;

      // Add the observation to the correct summary

        summary_values[summary_key]->add(curr_obs->getValue());
      }

      // Move to the next obs

      ++curr_obs;
    }

    // Calculate the summaries and add them to the summary observations list

    map< SummaryKey, NumArray* >::const_iterator curr_values;
    for (curr_values = summary_values.begin();
         curr_values != summary_values.end(); ++curr_values)
    {
      // Loop through the calculators, saving a summary for each one

      vector< SummaryCalc* >::const_iterator calc_iter;

      for (calc_iter = calculators.begin();
           calc_iter != calculators.end(); ++calc_iter)
      {
        SummaryCalc *calc = *calc_iter;

        // Compute the expected number of observations and check valid data ratio

        if (summary_info.vld_freq > 0 && summary_info.vld_thresh > 0)
        {
          int n_expect = max(1, nint(summary_info.width / summary_info.vld_freq));
          int n_valid  = (*curr_values->second).n_valid();

          if (((double) n_valid / n_expect) < summary_info.vld_thresh)
          {

            mlog << Debug(4)
                 << "Skipping time summary since the ratio of valid data "
                 << n_valid << "/" << n_expect << " < " << summary_info.vld_thresh
                 << " for " << curr_values->first.getHeaderType() << ", "
                 << calc->getType() << ", "
                 << summary_info.width << " seconds, "
                 << curr_values->first.getStationId() << ", "
                 << unix_to_yyyymmdd_hhmmss(time_interval->getBaseTime()) << ", "
                 << curr_values->first.getLatitude() << ", "
                 << curr_values->first.getLongitude() << ", "
                 << curr_values->first.getElevation() << ", "
                 << curr_values->first.getVarName() << ", "
                 << curr_values->first.getGribCode() << "\n";
            continue;
          }
        }

        summary_obs.push_back(Observation(_getSummaryHeaderType(curr_values->first.getHeaderType(),
                                                                calc->getType(),
                                                                summary_info.width),
                                          curr_values->first.getStationId(),
                                          time_interval->getBaseTime(),
                                          curr_values->first.getLatitude(),
                                          curr_values->first.getLongitude(),
                                          curr_values->first.getElevation(),
                                          "",
                                          curr_values->first.getGribCode(),
                                          curr_values->first.getPressureLevel(),
                                          curr_values->first.getHeight(),
                                          calc->calcSummary(*curr_values->second),
                                          curr_values->first.getVarName()));

      } /* endfor - calc */

    } /* endfor - curr_values */

    // Reclaim space for the summary arrays

    for (curr_values = summary_values.begin();
         curr_values != summary_values.end(); ++curr_values)
      delete curr_values->second;

  } /* endfor - time_interval */

  // Replace the observations vector with the summary observations

  _observations = summary_obs;
  for (int idx=0; idx<summary_vnames.n_elements(); idx++) {
    if (!obs_names.has(summary_vnames[idx])) obs_names.add(summary_vnames[idx]);
  }

  // Reclaim memory

  for (size_t i = 0; i < calculators.size(); ++i)
    delete calculators[i];

  return true;
}


////////////////////////////////////////////////////////////////////////
// Protected/Private Methods
////////////////////////////////////////////////////////////////////////

void FileHandler::_closeNetcdf()
{
   //_ncFile->close();
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
  double prev_latitude = bad_data_double;
  double prev_longitude = bad_data_double;
  double prev_elevation = bad_data_double;

  for (vector< Observation >::const_iterator obs = _observations.begin();
       obs != _observations.end(); ++obs)
  {
    if (obs->getHeaderType() != prev_header_type    ||
        obs->getStationId()  != prev_station_id     ||
        obs->getValidTime()  != prev_valid_time     ||
        !is_eq(obs->getLatitude(),  prev_latitude)  ||
        !is_eq(obs->getLongitude(), prev_longitude) ||
        !is_eq(obs->getElevation(), prev_elevation))
    {
      _nhdr++;

      prev_header_type = obs->getHeaderType();
      prev_station_id  = obs->getStationId();
      prev_valid_time  = obs->getValidTime();
      prev_latitude    = obs->getLatitude();
      prev_longitude   = obs->getLongitude();
      prev_elevation   = obs->getElevation();
    }

  } /* endfor - obs */

}

////////////////////////////////////////////////////////////////////////

vector< SummaryCalc* > FileHandler::_getSummaryCalculators(const TimeSummaryInfo &info) const
{
  // Initialize the list of calculators

  vector< SummaryCalc * > calculators;

  // Loop through the summary types, creating the calculators

  for (int i = 0; i < info.type.n_elements(); ++i)
  {
    // Convert the current type to a string for easier processing

    string type = info.type[i];

    // Create the calculator specified

    if (type == "mean")
    {
      calculators.push_back(new SummaryCalcMean);
    }
    else if (type == "stdev")
    {
      calculators.push_back(new SummaryCalcStdev);
    }
    else if (type == "min")
    {
      calculators.push_back(new SummaryCalcMin);
    }
    else if (type == "max")
    {
      calculators.push_back(new SummaryCalcMax);
    }
    else if (type == "range")
    {
      calculators.push_back(new SummaryCalcRange);
    }
    else if (type == "median")
    {
      calculators.push_back(new SummaryCalcMedian);
    }
    else if (type[0] == 'p')
    {
      calculators.push_back(new SummaryCalcPercentile(type));
    }
  }

  return calculators;
}

////////////////////////////////////////////////////////////////////////

string FileHandler::_getSummaryHeaderType(const string &header_type,
					  const string &summary_type,
					  const int summary_width_secs) const
{
  // Extract the time values from the width

  char header_type_string[1024];

  sprintf(header_type_string, "%s_%s_%s",
	  header_type.c_str(), summary_type.c_str(),
	  _secsToTimeString(summary_width_secs).c_str());

  return string(header_type_string);
}

////////////////////////////////////////////////////////////////////////

vector< TimeSummaryInterval > FileHandler::_getTimeIntervals(const time_t first_data_time,
                                                             const time_t last_data_time,
                                                             const TimeSummaryInfo &info) const
{
  // Add the time intervals based on the relationship between the begin and
  // end times.

  vector< TimeSummaryInterval > time_intervals;

  time_t interval_time = _getIntervalTime(first_data_time, info.beg, info.end,
                                          info.step, info.width);

  while (interval_time < last_data_time)
  {
    // We need to process each day separately so that we can always start
    // at the indicated start time on each day.

    time_t day_end_time = _getEndOfDay(interval_time);

    while (interval_time < day_end_time &&
           interval_time < last_data_time)
    {
      // See if the current time is within the defined time intervals

      if (_isInTimeInterval(interval_time, info.beg, info.end))
        time_intervals.push_back(TimeSummaryInterval(interval_time,
                                 info.width));

      // Increment the current time

      interval_time += info.step;
    }

  }

  return time_intervals;
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

bool FileHandler::_isInTimeInterval(const time_t test_time,
                                    const int begin_secs, const int end_secs) const
{
  // If the begin and end times are the same, assume the user wants all times

  if (begin_secs == end_secs)
    return true;

  // Extract the seconds from the test time

  int test_secs = _unixtimeToSecs(test_time);

  // Test for an interval that doesn't span midnight

  if (begin_secs < end_secs)
    return test_secs >= begin_secs && test_secs <= end_secs;

  // If we get here, the time interval spans midnight

  if (test_secs >= 0 && test_secs <= end_secs)
    return true;

  if (test_secs >= begin_secs && test_secs <= sec_per_day)
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::_openNetcdf(const string &nc_filename)
{
   mlog << Debug(1) << "Creating NetCDF Observation file: "
        << nc_filename << "\n";

   //
   // Create the output NetCDF file for writing
   //
   _ncFile = open_ncfile(nc_filename.c_str(), true);

   if(IS_INVALID_NC_P(_ncFile)) {
      mlog << Error << "\nFileHandler::_openNetcdf() -> "
           << "can't open output NetCDF file \"" << nc_filename
           << "\" for writing\n\n";
      _closeNetcdf();

      return false;
   }

   //
   // Define the NetCDF dimensions
   //
   NcDim strl_dim    = add_dim(_ncFile,"mxstr", MAX_STRING_LEN);
   NcDim hdr_arr_dim = add_dim(_ncFile,"hdr_arr_len", HDR_ARRAY_LEN);
   NcDim obs_arr_dim = add_dim(_ncFile,"obs_arr_len", OBS_ARRAY_LEN);
   NcDim hdr_dim     = add_dim(_ncFile,"nhdr", _nhdr);
   NcDim obs_dim     = add_dim(_ncFile,"nobs"); // unlimited dimension

   //
   // Add variables to NetCDF file
   //
   //mlog << Debug(4) << "\nFileHandler::_openNetcdf() compression_level: " << deflate_level << "\n";
   _hdrTypeVar      = add_var(_ncFile, "hdr_typ", ncChar,  hdr_dim, strl_dim,    deflate_level);
   _hdrStationIdVar = add_var(_ncFile, "hdr_sid", ncChar,  hdr_dim, strl_dim,    deflate_level);
   _hdrValidTimeVar = add_var(_ncFile, "hdr_vld", ncChar,  hdr_dim, strl_dim,    deflate_level);
   _hdrArrayVar     = add_var(_ncFile, "hdr_arr", ncFloat, hdr_dim, hdr_arr_dim, deflate_level);
   _obsQualityVar   = add_var(_ncFile, "obs_qty", ncChar,  obs_dim, strl_dim,    deflate_level);
   _obsArrayVar     = add_var(_ncFile, "obs_arr", ncFloat, obs_dim, obs_arr_dim, deflate_level);

   //
   // Add attributes to the NetCDF variables
   //

   add_att(&_hdrTypeVar,"long_name", "message type");
   add_att(&_hdrStationIdVar,"long_name", "station identification");
   add_att(&_hdrValidTimeVar,"long_name", "valid time");
   add_att(&_hdrValidTimeVar,"units", "YYYYMMDD_HHMMSS UTC");

   add_att(&_hdrArrayVar, "long_name", "array of observation station header values");
   add_att(&_hdrArrayVar, "missing_value", FILL_VALUE);
   add_att(&_hdrArrayVar, "_FillValue", FILL_VALUE);
   add_att(&_hdrArrayVar, "columns", "lat lon elv");
   add_att(&_hdrArrayVar, "lat_long_name", "latitude");
   add_att(&_hdrArrayVar, "lat_units", "degrees_north");
   add_att(&_hdrArrayVar, "lon_long_name", "longitude");
   add_att(&_hdrArrayVar, "lon_units", "degrees_east");
   add_att(&_hdrArrayVar, "elv_long_name", "elevation ");
   add_att(&_hdrArrayVar, "elv_units", "meters above sea level (msl)");

   add_att(&_obsQualityVar, "long_name", "quality flag");

   add_att(&_obsArrayVar, "long_name", "array of observation values");
   add_att(&_obsArrayVar, "missing_value", FILL_VALUE);
   add_att(&_obsArrayVar, "_FillValue", FILL_VALUE);
   add_att(&_obsArrayVar, "hdr_id_long_name", "index of matching header data");
   if (use_var_id) {
      add_att(&_obsArrayVar, "columns", "hdr_id var_id lvl hgt ob");
      add_att(&_obsArrayVar, "var_id_long_name", "index of variable names at var_name");
   }
   else {
      add_att(&_obsArrayVar, "columns", "hdr_id gc lvl hgt ob");
      add_att(&_obsArrayVar, "gc_long_name", "grib code corresponding to the observation type");
   }
   add_att(&_obsArrayVar, "lvl_long_name", "pressure level (hPa) or accumulation interval (sec)");
   add_att(&_obsArrayVar, "hgt_long_name", "height in meters above sea level or ground level (msl or agl)");
   add_att(&_obsArrayVar, "ob_long_name", "observation value");

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
  int hdr_sid_len;

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
   hdr_arr_buf[hdr_data_idx][0] = lat;
   hdr_arr_buf[hdr_data_idx][1] = lon;
   hdr_arr_buf[hdr_data_idx][2] = elv;

   if(hdr_sid.length() > MAX_STRING_LEN) {
      mlog << Warning << "\nFileHandler::_writeHdrInfo() -> "
           << "only writing the first " << MAX_STRING_LEN
           << " of station id: " << hdr_sid << "\n\n";
      hdr_sid_len = MAX_STRING_LEN;
   }
   else {
      hdr_sid_len = hdr_sid.length();
   }

   if(check_reg_exp(yyyymmdd_hhmmss_reg_exp, hdr_vld) != true) {
      mlog << Error << "\nFileHandler::_writeHdrInfo() -> "
           << "valid time is not in the expected YYYYMMDD_HHMMSS format: "
           << hdr_vld << "\n\n";
      return false;
   }
   strncpy(&hdr_typ_buf[hdr_data_idx][0], hdr_typ, hdr_typ.length());
   hdr_typ_buf[hdr_data_idx][hdr_typ.length()] = bad_data_char;
   strncpy(&hdr_sid_buf[hdr_data_idx][0], hdr_sid, hdr_sid_len);
   hdr_sid_buf[hdr_data_idx][hdr_sid_len] = bad_data_char;
   strncpy(&hdr_vld_buf[hdr_data_idx][0], hdr_vld, hdr_vld.length());
   hdr_vld_buf[hdr_data_idx][hdr_vld.length()] = bad_data_char;
   for (int j=hdr_typ.length(); j<MAX_STRING_LEN; j++)
       hdr_typ_buf[hdr_data_idx][j] = bad_data_char;

   hdr_data_idx++;


   hdr_buf_size = _nhdr;
   if (hdr_buf_size > OBS_BUFFER_SIZE) hdr_buf_size = OBS_BUFFER_SIZE;

   bool save_nc = (hdr_buf_size <= hdr_data_idx);
   if (save_nc) {

      long offsets[2] = { hdr_data_offset, 0 };
      long lengths[2] = { hdr_buf_size, MAX_STRING_LEN };

      hdr_data_offset += hdr_buf_size;

      //
      // Store the message type
      //
      if(!put_nc_data(&_hdrTypeVar, (char *)&hdr_typ_buf[0], lengths, offsets)) {
         mlog << Error << "\nFileHandler::_writeHdrInfo() -> "
              << "error writing the message type to the NetCDF file\n\n";
         return false;
      }

      //
      // Store the station id
      //

      if(!put_nc_data(&_hdrStationIdVar, (char *)&hdr_sid_buf[0], lengths, offsets)) {
         mlog << Error << "\nFileHandler::_writeHdrInfo() -> "
              << "error writing the station id to the NetCDF file\n\n";
         return false;
      }

      //
      // Store the valid time and check that it's is in the expected
      // time format: YYYYMMDD_HHMMSS
      //
      if(!put_nc_data(&_hdrValidTimeVar, (char *)&hdr_vld_buf[0], lengths, offsets)) {
         mlog << Error << "\nFileHandler::_nwriteHdrInfo() -> "
              << "error writing the valid time to the NetCDF file\n\n";
         return false;
      }

      //
      // Store the header array
      //
      lengths[1] = HDR_ARRAY_LEN;
      if(!put_nc_data(&_hdrArrayVar, (float *)&hdr_arr_buf[0], lengths, offsets) ) {
         mlog << Error << "\nFileHandler::_writeHdrInfo() -> "
              << "error writing the header array to the NetCDF file\n\n";
         return false;
      }

      hdr_data_idx = 0;
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::_writeObsInfo(int gc, float prs, float hgt, float obs,
                                const ConcatString &qty) {
   float obs_arr[OBS_ARRAY_LEN];
   ConcatString obs_qty;

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
   for (int idx=0; idx<OBS_ARRAY_LEN; idx++) {
      obs_data_buf[obs_data_idx][idx] = obs_arr[idx];
   }

   obs_qty = (qty.length() == 0 ? na_str : qty.text());
   strncpy(qty_data_buf[obs_data_idx], obs_qty, obs_qty.length());

   if ( _observations.size() == _obsNum && OBS_BUFFER_SIZE == obs_buf_size) {
      obs_buf_size = _observations.size() % OBS_BUFFER_SIZE;
   }
   bool save_nc = (obs_buf_size <= (obs_data_idx+1));
   if (! save_nc && processed_count == _observations.size() ) {
      save_nc = true;
      obs_buf_size = _observations.size() % OBS_BUFFER_SIZE;
   }
   if (save_nc) {
      long offsets[2] = { obs_data_offset, 0 };
      long lengths[2] = { obs_buf_size, OBS_ARRAY_LEN };
      obs_data_offset += obs_buf_size;

      //
      // Write the observation array
      //
      if(!put_nc_data(&_obsArrayVar, (float *)obs_data_buf, lengths, offsets) ) {
         mlog << Error << "\nFileHandler::_writeObsInfo() -> "
              << "error writing the observation array to the NetCDF file\n\n";
         return false;
      }

      //
      // Write the observation QC flag, resetting an empty string to NA
      //
      lengths[1] = MAX_STRING_LEN;
      if(!put_nc_data(&_obsQualityVar, (char *)qty_data_buf, lengths, offsets) ) {
         mlog << Error << "\nFileHandler::_writeObsInfo() -> "
              << "error writing the quality flag to the NetCDF file\n\n";
         return false;
      }
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::_addObservations(const Observation &obs)
{
   double grid_x, grid_y;

   //
   // Apply the grid mask
   //
   if(_gridMask)
   {
     _gridMask->latlon_to_xy(obs.getLatitude(), -1.0*obs.getLongitude(),
                             grid_x, grid_y);

     if(grid_x < 0 || grid_x >= _gridMask->nx() ||
        grid_y < 0 || grid_y >= _gridMask->ny()) {
        _gridMaskNum++;
        return false;
     }
   }

   //
   // Apply the polyline mask
   //
   if(_polyMask)
   {
     if(!_polyMask->latlon_is_inside_dege(obs.getLatitude(), obs.getLongitude()))
     {
       _polyMaskNum++;
       return false;
     }
   }

   //
   // Apply the station ID mask
   //
   if(_sidMask)
   {
     if(!_sidMask->has(obs.getStationId().c_str()))
     {
       _sidMaskNum++;
       return false;
     }
   }

   _observations.push_back(obs);
   
   const char *var_name = obs.getVarName().c_str();
   if (0 < strlen(var_name) && !obs_names.has(var_name)) {
      obs_names.add(var_name);
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::_writeObservations()
{
  int grib_code;
  string prev_header_type = "";
  string prev_station_id = "";
  time_t prev_valid_time = 0;
  double prev_latitude = bad_data_double;
  double prev_longitude = bad_data_double;
  double prev_elevation = bad_data_double;

  obs_buf_size = _observations.size();
  if (obs_buf_size > OBS_BUFFER_SIZE) obs_buf_size = OBS_BUFFER_SIZE;

  obs_data_idx = 0;
  obs_data_offset = 0;
  hdr_data_idx = 0;
  hdr_data_offset = 0;

  processed_count =0;
  for (vector< Observation >::const_iterator obs = _observations.begin();
       obs != _observations.end(); ++obs)
  {

    processed_count++;

    if (obs->getHeaderType() != prev_header_type    ||
        obs->getStationId()  != prev_station_id     ||
        obs->getValidTime()  != prev_valid_time     ||
        !is_eq(obs->getLatitude(),  prev_latitude)  ||
        !is_eq(obs->getLongitude(), prev_longitude) ||
        !is_eq(obs->getElevation(), prev_elevation))
    {
      if (!_writeHdrInfo(obs->getHeaderType().c_str(),
                         obs->getStationId().c_str(),
                         obs->getValidTimeString().c_str(),
                         obs->getLatitude(),
                         obs->getLongitude(),
                         obs->getElevation()))
        return false;

      prev_header_type = obs->getHeaderType();
      prev_station_id  = obs->getStationId();
      prev_valid_time  = obs->getValidTime();
      prev_latitude    = obs->getLatitude();
      prev_longitude   = obs->getLongitude();
      prev_elevation   = obs->getElevation();
    }

    if (!_writeObsInfo(obs->getGribCode(),
                       obs->getPressureLevel(),
                       obs->getHeight(),
                       obs->getValue(),
                       obs->getQualityFlag().c_str()))
      return false;

    obs_data_idx++;
    if (obs_data_idx >= obs_buf_size) obs_data_idx = 0;

  } /* endfor - obs */

  return true;
}

