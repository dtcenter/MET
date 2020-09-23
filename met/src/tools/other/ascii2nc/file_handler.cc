// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
  _areaMaskNum(0),
  _polyMaskNum(0),
  _sidMaskNum(0),
  _gridMask(0),
  _areaMask(0),
  _polyMask(0),
  _sidMask(0),
  use_var_id(false),
  do_monitor(false),
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

    if (!ascii_file.open((*ascii_filename).c_str()))
    {
      mlog << Error << "\nFileHandler::readAsciiFiles() -> "
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
       << "Rejected " << _areaMaskNum + _polyMaskNum
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

  if (_dataSummarized) write_summary_attributes(_ncFile, _summaryInfo);

  // Write the headers and observations to the netCDF file.

  if (!_writeObservations())
    return false;

  // Add variable names
  if (use_var_id) write_obs_var_names(obs_vars, obs_names);

  // Close the netCDF file.

  _closeNetcdf();

  mlog << Debug(2) << "Finished processing " << obs_vars.obs_cnt
       << " observations for " << obs_vars.hdr_cnt << " headers.\n";

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
   int raw_header_count = summary_obs.countHeaders(_observations);
   _nhdr = (do_summary ? summary_obs.countSummaryHeaders()
                       : raw_header_count);
   if (do_summary && _summaryInfo.raw_data) {
      _nhdr += raw_header_count;
   }
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
   init_nc_dims_vars_config(obs_vars, use_var_id);
   obs_vars.attr_agl   = true;

   nc_out_data.processed_hdr_cnt = 0;
   nc_out_data.deflate_level = deflate_level;
   nc_out_data.observations = _observations;
   nc_out_data.summary_obs = &summary_obs;
   nc_out_data.summary_info = _summaryInfo;

   init_netcdf_output(_ncFile, obs_vars, nc_out_data, _programName);

   //
   // Initialize the header and observation record counters
   //
   _hdrNum = -1;
   _obsNum = -1;

   return true;
}


////////////////////////////////////////////////////////////////////////

//bool FileHandler::_writeHdrInfo(const ConcatString &hdr_typ,
//                                const ConcatString &hdr_sid,
//                                const time_t hdr_vld,
//                                double lat, double lon, double elv) {
//   //
//   // Increment header count before writing
//   //
//   _hdrNum++;
//   write_nc_header(obs_vars, hdr_typ, hdr_sid, hdr_vld, lat, lon, elv);
//
//   return true;
//}

////////////////////////////////////////////////////////////////////////

//bool FileHandler::_writeObsInfo(int gc, float prs, float hgt, float obs,
//                                const ConcatString &qty) {
//   float obs_arr[OBS_ARRAY_LEN];
//   ConcatString obs_qty;
//
//   //
//   // Increment observation count before writing
//   //
//   _obsNum++;
//
//   //
//   // Build the observation array
//   //
//   obs_arr[0] = _hdrNum; // Index of header
//   obs_arr[1] = gc;    // GRIB code corresponding to the observation type
//   obs_arr[2] = prs;   // Pressure level (hPa) or accumulation interval (sec)
//   obs_arr[3] = hgt;   // Height in meters above sea level or ground level (msl or agl)
//   obs_arr[4] = obs;   // Observation value
//
//   obs_qty = (qty.length() == 0 ? na_str : qty.text());
//   write_nc_observation(obs_vars, nc_data_buffer, obs_arr, obs_qty.text());
//
//   return true;
//}

////////////////////////////////////////////////////////////////////////

bool FileHandler::_addObservations(const Observation &obs)
{
   double grid_x, grid_y;

   //
   // Apply the grid mask
   //
   if(_gridMask) {
      _gridMask->latlon_to_xy(obs.getLatitude(), -1.0*obs.getLongitude(),
                              grid_x, grid_y);

      if(grid_x < 0 || grid_x >= _gridMask->nx() ||
         grid_y < 0 || grid_y >= _gridMask->ny()) {
         _gridMaskNum++;
         return false;
      }

      //
      // Apply the area mask
      //
      if(_areaMask) {
         if(!_areaMask->s_is_on(nint(grid_x), nint(grid_y))) {
            _areaMaskNum++;
            return false;
         }
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

   // Save obs because the obs vector is sorted after time summary
   _observations.push_back(obs);
   if (do_summary) summary_obs.addObservationObj(obs);
   else {
      ConcatString var_name = obs.getVarName();
      if (var_name.nonempty() && !obs_names.has(var_name)) {
         obs_names.add(var_name);
      }
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

bool FileHandler::_writeObservations()
{
  write_observations(_ncFile, obs_vars, nc_out_data);

  int var_count = (use_var_id ? obs_names.n_elements() : 0);
  if (var_count > 0) {
    int unit_count = 0;
    create_nc_obs_name_vars (obs_vars, _ncFile, var_count, unit_count, deflate_level);
    write_obs_var_names(obs_vars, obs_names);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////

void FileHandler::debug_print_observations(vector< Observation > my_observation, string extra_str) {
  int count = 0;
  int obs_count = 0;
  int threshold_count = 10;
  int prev_hdr_idx = -1;
  string methd_name = "FileHandler::debug_print_observations()   ";
  cout << methd_name << extra_str << "  count: " << (int)my_observation.size() << "\n";
  for (vector< Observation >::const_iterator obs = my_observation.begin();
       obs != my_observation.end(); ++obs)
  {
    obs_count++;
    if (count > threshold_count && prev_hdr_idx == obs->getHeaderIndex()) continue;
    cout << methd_name << extra_str << "      obs index: " << (obs_count - 1)
         << "  header index: " << obs->getHeaderIndex()
         << "  Sid: " << obs->getStationId()
         << "  lat: " << obs->getLatitude() << " lon: " << obs->getLongitude()
         << "  vld time: " << obs->getValidTimeString()
         << "  GC/VarIdx: " << obs->getGribCode() << "  Value: " << obs->getValue()
         << "  HeaderType: " << obs->getHeaderType() << "\n";
    count++;
    if (threshold_count < obs->getHeaderIndex()) break;
    prev_hdr_idx = obs->getHeaderIndex();
  }
  Observation last_obs = my_observation.at(my_observation.size()-1);
  cout << methd_name << extra_str << " last obs index: " << (obs_count - 1)
       << "  header index: " << last_obs.getHeaderIndex()
       << "  Sid: " << last_obs.getStationId()
       << "  lat: " << last_obs.getLatitude() << " lon: " << last_obs.getLongitude()
       << "  vld time: " << last_obs.getValidTimeString()
       << "  GC/VarIdx: " << last_obs.getGribCode() << "  Value: " << last_obs.getValue()
       << "  HeaderType: " << last_obs.getHeaderType() << "\n";
}
