// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
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

extern struct NcDataBuffer nc_data_buffer;  // at write_netcdf.cc
extern struct NcHeaderData hdr_data;        // at write_netcdf.cc

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
  if (_ncFile != 0) delete _ncFile;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::readAsciiFiles(const vector< ConcatString > &ascii_filename_list)
{
  nc_obs_initialize();
  
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
    int max_name_len = HEADER_STR_LEN;
    char var_name[max_name_len];
    add_att(_ncFile, nc_att_use_var_id, "true");
    NcDim var_dim     = add_dim(_ncFile, nc_dim_nvar, (long)obs_names.n_elements());
    NcDim strl_dim    = get_nc_dim(_ncFile,  nc_dim_mxstr);
    NcVar var_obs_var = add_var(_ncFile, nc_var_obs_var, ncChar, var_dim, strl_dim, deflate_level);
    add_att(&var_obs_var,  "long_name", "variable names from ASCII input");
    
    long offsets[2] = { 0, 0 };
    long lengths[2] = { 1, max_name_len } ;
    for(int i=0; i<obs_names.n_elements(); i++) {
      for(int tIdx=0; tIdx<max_name_len; tIdx++) {
        var_name[tIdx] = bad_data_char;
      }
      strcpy(var_name, obs_names[i]);

      if(!put_nc_data(&var_obs_var, (char *)var_name, lengths, offsets)) {
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

void FileHandler::setSummaryInfo(const TimeSummaryInfo &summary_info) {
   do_summary = summary_info.flag; 
   _summaryInfo = summary_info;
   summary_obs.setSummaryInfo(summary_info);
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::summarizeObs(const TimeSummaryInfo &summary_info)
{
   bool result = summary_obs.summarizeObs(summary_info);
   
   //_observations = summary_obs.getSummaries();
   
   _dataSummarized = true;
   _summaryInfo = summary_info;
   StringArray summary_vnames = summary_obs.getObsNames();
   for (int idx=0; idx<summary_vnames.n_elements(); idx++) {
      if (!obs_names.has(summary_vnames[idx])) obs_names.add(summary_vnames[idx]);
   }
   return result;
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
   _nhdr = (do_summary ? summary_obs.countSummaryHeaders()
                       : summary_obs.countHeaders());
}

////////////////////////////////////////////////////////////////////////

time_t FileHandler::_getValidTime(const string &time_string) const
{
   return summary_obs.getValidTime(time_string);
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
   // Define the NetCDF dimensions and variables
   //
   init_nc_dims_vars (obs_vars, use_var_id);
   obs_vars.attr_agl   = true;

   create_nc_hdr_vars(obs_vars, _ncFile, _nhdr, deflate_level);
   create_nc_obs_vars(obs_vars, _ncFile, deflate_level, use_var_id);

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
   //
   // Increment header count before writing
   //
   _hdrNum++;
   write_nc_header(obs_vars, hdr_typ, hdr_sid, hdr_vld, lat, lon, elv);

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
   
   obs_qty = (qty.length() == 0 ? na_str : qty.text());
   write_nc_observation(obs_vars, nc_data_buffer, obs_arr, obs_qty.text());
   
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

   summary_obs.addObservationObj(obs);
   //   _observations.push_back(obs);
   
   if (!do_summary) {
      const char *var_name = obs.getVarName().c_str();
      if (0 < strlen(var_name) && !obs_names.has(var_name)) {
         obs_names.add(var_name);
      }
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::_writeObservations()
{
  string prev_header_type = "";
  string prev_station_id = "";
  time_t prev_valid_time = 0;
  double prev_latitude = bad_data_double;
  double prev_longitude = bad_data_double;
  double prev_elevation = bad_data_double;

  if (do_summary) {
    write_nc_observations(obs_vars, summary_obs.getSummaries());
    if (IS_INVALID_NC(obs_vars.hdr_arr_var) || IS_INVALID_NC(obs_vars.hdr_lat_var)) {
      create_nc_other_vars (obs_vars, _ncFile, nc_data_buffer, hdr_data);
      write_nc_headers(obs_vars);
    }
    else {
      write_nc_header(obs_vars);
    }
  }
  else {
    _observations = summary_obs.getObservations();
    
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
      
    } /* endfor - obs */
    
    write_nc_observation(obs_vars, nc_data_buffer);

    int var_count = 0;
    int unit_count = 0;
    create_nc_other_vars (obs_vars, _ncFile, nc_data_buffer, hdr_data, var_count, unit_count, deflate_level);
    write_nc_header(obs_vars);
  }
  return true;
}

