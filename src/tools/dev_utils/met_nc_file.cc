// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


using namespace std;

#include "vx_log.h"
#include "vx_math.h"

#include "met_nc_file.h"
#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetNcFile
   //


////////////////////////////////////////////////////////////////////////


MetNcFile::MetNcFile(const string &file_path) :
  _filePath(file_path),
  _ncFile(0),
  _hdrArrDim(0),
  _obsArrDim(0),
  _nhdrDim(0),
  _nobsDim(0),
  _strlDim(0),
  _hdrArrVar(0),
  _hdrTypeVar(0),
  _hdrSidVar(0),
  _hdrVldVar(0),
  _obsArrVar(0)
{
}


////////////////////////////////////////////////////////////////////////


MetNcFile::~MetNcFile()
{
  // Do nothing
}


////////////////////////////////////////////////////////////////////////


bool MetNcFile::readFile(const int desired_grib_code,
			 const string &desired_station_id,
			 const string &desired_message_type,
			 vector< SDObservation > &observations)
{
  static const string method_name = "MetNcFile::readFile()";

  // Open the netCDF point observation file

  mlog << Debug(1) << "Opening netCDF file: " << _filePath << "\n";

  _ncFile = open_ncfile(_filePath.c_str());

  if (!_ncFile || !_ncFile->is_valid())
  {
    mlog << Error << "\n" << method_name << " -> trouble opening netCDF file "
	 << _filePath << "\n\n";
    _ncFile->close();
    delete _ncFile;
    _ncFile = (NcFile *) 0;

    return false;
  }

  // Retrieve the dimensions and variable from the netCDF file

  _hdrArrDim = _ncFile->get_dim("hdr_arr_len");
  _obsArrDim = _ncFile->get_dim("obs_arr_len");

  _nhdrDim = _ncFile->get_dim("nhdr");
  _nobsDim = _ncFile->get_dim("nobs");

  _strlDim = _ncFile->get_dim("mxstr");

  _hdrArrVar = _ncFile->get_var("hdr_arr");
  _hdrTypeVar = _ncFile->get_var("hdr_typ");
  _hdrSidVar = _ncFile->get_var("hdr_sid");
  _hdrVldVar = _ncFile->get_var("hdr_vld");
  _obsArrVar = _ncFile->get_var("obs_arr");

  if (!_hdrArrDim || !_hdrArrDim->is_valid() ||
      !_obsArrDim || !_obsArrDim->is_valid() ||
      !_nhdrDim    || !_nhdrDim->is_valid()    ||
      !_nobsDim    || !_nobsDim->is_valid()    ||
      !_strlDim    || !_strlDim->is_valid()    ||
      !_hdrArrVar || !_hdrArrVar->is_valid() ||
      !_hdrTypeVar || !_hdrTypeVar->is_valid() ||
      !_hdrSidVar || !_hdrSidVar->is_valid() ||
      !_hdrVldVar || !_hdrVldVar->is_valid() ||
      !_obsArrVar || !_obsArrVar->is_valid())
  {
    mlog << Error << "\nmain() -> "
	 << "trouble reading netCDF file " << _filePath << "\n\n";
    return false;
  }

  // Allocate space to store the data

  float *obs_arr = new float[_obsArrDim->size()];

  mlog << Debug(2) << "Processing " << _nobsDim->size() << " observations at "
       << _nhdrDim->size() << " locations.\n";

  // Loop through the observations, saving the ones that we are
  // interested in

  for (int i = 0; i < _nobsDim->size(); ++i)
  {
    if (!_obsArrVar->set_cur(i, 0) ||
	!_obsArrVar->get(obs_arr, 1, _obsArrDim->size()))
    {
      mlog << Error << "\n" << method_name << " -> trouble getting obs_arr\n\n";
      return false;
    }

    if (obs_arr[0] >= 1.0E10 && obs_arr[1] >= 1.0E10)
      break;

    // Get the header index and variable type for this observation.

    int hdr_index = nint(obs_arr[0]);
    int grib_code = nint(obs_arr[1]);

    // Check if we want to plot this variable type.

    if (grib_code != desired_grib_code)
      continue;

    // Get the corresponding header message type

    char message_type_buffer[max_str_len];
    if (!_hdrTypeVar->set_cur(hdr_index, 0) ||
	!_hdrTypeVar->get(message_type_buffer, 1, _strlDim->size()))
    {
      mlog << Error << "\n" << method_name << " -> "
	   << "trouble getting hdr_typ\n\n";
      return false;
    }
    string message_type = message_type_buffer;

    if (message_type != desired_message_type)
      continue;

    // Get the corresponding header station id

    char station_id_buffer[max_str_len];
    if (!_hdrSidVar->set_cur(hdr_index, 0) ||
	!_hdrSidVar->get(station_id_buffer, 1, _strlDim->size()))
    {
      mlog << Error << "\n" << method_name << " -> "
	   << "trouble getting hdr_sid\n\n";
      return false;
    }
    string station_id = station_id_buffer;

    if (station_id != desired_station_id)
      continue;

    // Get the corresponding header valid time

    char hdr_vld_buffer[max_str_len];

    if (!_hdrVldVar->set_cur(hdr_index, 0) ||
	!_hdrVldVar->get(hdr_vld_buffer, 1, _strlDim->size()))
    {
      mlog << Error << "\n" << method_name << " -> "
	   << "trouble getting hdr_vld\n\n";
      return false;
    }

    // If we get here, this is an observation that we want to use

    SDObservation obs(hdr_vld_buffer, obs_arr[4]);

    observations.push_back(obs);
   } // end for i

  // Cleanup

  if (obs_arr) { delete obs_arr; obs_arr = (float *) 0; }

  return true;
}


////////////////////////////////////////////////////////////////////////
// Protected/Private Methods
////////////////////////////////////////////////////////////////////////

