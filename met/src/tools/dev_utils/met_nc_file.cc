// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

  if (!_ncFile || IS_INVALID_NC_P(_ncFile))
  {
    mlog << Error << "\n" << method_name << " -> trouble opening netCDF file "
	 << _filePath << "\n\n";
    //_ncFile->close();
    delete _ncFile;
    _ncFile = (NcFile *) 0;

    return false;
  }

  // Retrieve the dimensions and variable from the netCDF file

  hdrArrDim = get_nc_dim(_ncFile, "hdr_arr_len");
  obsArrDim = get_nc_dim(_ncFile, "obs_arr_len");

  nhdrDim = get_nc_dim(_ncFile, "nhdr");
  nobsDim = get_nc_dim(_ncFile, "nobs");

  strlDim  = get_nc_dim(_ncFile, "mxstr");
  strllDim = get_nc_dim(_ncFile, "mxstr2");

  _hdrArrDim = &hdrArrDim;
  _obsArrDim = &obsArrDim;
  _nhdrDim   = &nhdrDim;
  _nobsDim   = &nobsDim;
  _strlDim   = &strlDim;
  
  hdrArrVar  = get_nc_var(_ncFile, "hdr_arr");
  hdrTypeVar = get_nc_var(_ncFile, "hdr_typ");
  hdrSidVar  = get_nc_var(_ncFile, "hdr_sid");
  hdrVldVar  = get_nc_var(_ncFile, "hdr_vld");
  obsArrVar  = get_nc_var(_ncFile, "obs_arr");
  
  _hdrArrVar  = &hdrArrVar ;
  _hdrTypeVar = &hdrTypeVar;
  _hdrSidVar  = &hdrSidVar ;
  _hdrVldVar  = &hdrVldVar ;
  _obsArrVar  = &obsArrVar ;

  if (!_hdrArrDim  || IS_INVALID_NC_P(_hdrArrDim)  ||
      !_obsArrDim  || IS_INVALID_NC_P(_obsArrDim)  ||
      !_nhdrDim    || IS_INVALID_NC_P(_nhdrDim)    ||
      !_nobsDim    || IS_INVALID_NC_P(_nobsDim)    ||
      !_strlDim    || IS_INVALID_NC_P(_strlDim)    ||
      IS_INVALID_NC(hdrArrVar)  ||
      IS_INVALID_NC(hdrTypeVar) ||
      IS_INVALID_NC(hdrSidVar)  ||
      IS_INVALID_NC(hdrVldVar)  ||
      IS_INVALID_NC(obsArrVar))
  {
    mlog << Error << "\nmain() -> "
	 << "trouble reading netCDF file " << _filePath << "\n\n";
    return false;
  }

  // Allocate space to store the data
  long hdr_count   = get_dim_size(&nhdrDim);
  long obs_count   = get_dim_size(&nobsDim);
  long obs_arr_len = get_dim_size(&obsArrDim);
  long hdr_arr_len = get_dim_size(&hdrArrDim);
  long  strl_count = get_dim_size(&strlDim);
  long strll_count = strl_count;
  if (!IS_INVALID_NC(strllDim)) strll_count = get_dim_size(&strllDim);
  int typ_len = strl_count;
  int sid_len = strl_count;
  int vld_len = strl_count;
  if (!IS_INVALID_NC(strllDim)) {
    NcDim str_dim;
    string dim_name = GET_NC_NAME(strllDim);
    str_dim = get_nc_dim(&hdrTypeVar, dim_name);
    if (!IS_INVALID_NC(str_dim)) typ_len = strll_count;
    str_dim = get_nc_dim(&hdrSidVar, dim_name);
    if (!IS_INVALID_NC(str_dim)) sid_len = strll_count;
    str_dim = get_nc_dim(&hdrVldVar, dim_name);
    if (!IS_INVALID_NC(str_dim)) vld_len = strll_count;
  }

  float *obs_arr = new float[obs_arr_len];
  float *hdr_arr = new float[hdr_arr_len];

  mlog << Debug(2) << "Processing " << (obs_count) << " observations at "
       << (hdr_count) << " locations.\n";

  // Loop through the observations, saving the ones that we are
  // interested in

  

  //int buf_size = ((nobs_count > DEF_NC_BUFFER_SIZE) ? DEF_NC_BUFFER_SIZE : (nobs_count));
  int buf_size = obs_count;
  int hdr_buf_size = hdr_count;
  
  //
  // Allocate space to store the data
  //
  
  char hdr_typ_str_full[hdr_buf_size][typ_len];
  char hdr_sid_str_full[hdr_buf_size][sid_len];
  char hdr_vld_str_full[hdr_buf_size][vld_len];
  //float **hdr_arr_full = (float **) 0, **obs_arr_block = (float **) 0;

  float hdr_arr_full[hdr_buf_size][hdr_arr_len];
  float obs_arr_block[    buf_size][obs_arr_len];
  //char obs_qty_str_block[ buf_size][strl_count];

  long offsets[2] = { 0, 0 };
  long lengths[2] = { 1, 1 };
  
  lengths[0] = hdr_buf_size;
  lengths[1] = strl_count;

  //
  // Get the corresponding header message type
  //
  lengths[1] = typ_len;
  if(!get_nc_data(&hdrTypeVar, (char *)&hdr_typ_str_full[0], lengths, offsets)) {
    mlog << Error << "\nmain() -> "
         << "trouble getting hdr_typ\n\n";
    exit(1);
  }

  //
  // Get the corresponding header station id
  //
  lengths[1] = sid_len;
  if(!get_nc_data(&hdrSidVar, (char *)&hdr_sid_str_full[0], lengths, offsets)) {
    mlog << Error << "\nmain() -> "
         << "trouble getting hdr_sid\n\n";
    exit(1);
  }

  //
  // Get the corresponding header valid time
  //
  lengths[1] = vld_len;
  if(!get_nc_data(&hdrVldVar, (char *)&hdr_vld_str_full[0], lengths, offsets)) {
    mlog << Error << "\nmain() -> "
         << "trouble getting hdr_vld\n\n";
    exit(1);
  }

  //
  // Get the header for this observation
  //
  lengths[1] = hdr_arr_len;
  if(!get_nc_data(&hdrArrVar, (float *)&hdr_arr_full[0], lengths, offsets)) {
    mlog << Error << "\nmain() -> "
        << "trouble getting hdr_arr\n\n";
    exit(1);
  }
  
  //for(int i_start=0; i_start<nobs_count; i_start+=buf_size) {
  //   buf_size = ((nobs_count-i_start) > DEF_NC_BUFFER_SIZE) ? DEF_NC_BUFFER_SIZE : (nobs_count-i_start);
     
  offsets[0] = 0;
  lengths[0] = buf_size;
  lengths[1] = obs_arr_len;

  // Read the current observation message
  if(!get_nc_data(&obsArrVar, (float *)&obs_arr_block[0], lengths, offsets)) {
    mlog << Error << "\nmain() -> trouble getting obs_arr\n\n";
    exit(1);
  }

  lengths[1] = strl_count;
  //if(!get_nc_data(&obs_arr_var, (char *)&obs_qty_str_block[0], lengths, offsets)) {
  //   mlog << Error << "\nmain() -> trouble getting obs_arr\n\n";
  //   exit(1);
  //}
      
  for (unsigned int i = 0; i < GET_NC_SIZE_P(_nobsDim); ++i)
  {
      
    // Copy the current observation message
    for (int k=0; k < obs_arr_len; k++)
       obs_arr[k] = obs_arr_block[i][k];

    if (obs_arr[0] >= 1.0E10 && obs_arr[1] >= 1.0E10)
      break;

    // Read the current observation quality flag
    //strncpy(obs_qty_str, obs_qty_str_block[i_offset], strl_count);
      

    // Get the header index and variable type for this observation.

    int hdr_index = nint(obs_arr[0]);
    int grib_code = nint(obs_arr[1]);

    // Check if we want to plot this variable type.

    if (grib_code != desired_grib_code)
      continue;

    // Get the corresponding header message type
    // Read the corresponding header array for this observation
    for (int k=0; k < obs_arr_len; k++)
       hdr_arr[k] = hdr_arr_full[hdr_index][k];
   
    int  str_length;
    char message_type_buffer[max_str_len];
    char station_id_buffer[max_str_len];
    char hdr_vld_buffer[max_str_len];
    // Read the corresponding header type for this observation
    str_length = strlen(hdr_typ_str_full[hdr_index]);
    if (str_length > typ_len) str_length = typ_len;
    strncpy(message_type_buffer, hdr_typ_str_full[hdr_index], str_length);
    message_type_buffer[str_length] = bad_data_char;

    // Read the corresponding header Station ID for this observation
    str_length = strlen(hdr_sid_str_full[hdr_index]);
    if (str_length > sid_len) str_length = sid_len;
    strncpy(station_id_buffer, hdr_sid_str_full[hdr_index], str_length);
    station_id_buffer[str_length] = bad_data_char;

    // Read the corresponding valid time for this observation
    str_length = strlen(hdr_vld_str_full[hdr_index]);
    if (str_length > vld_len) str_length = vld_len;
    strncpy(hdr_vld_buffer, hdr_vld_str_full[hdr_index], str_length);
    hdr_vld_buffer[str_length] = bad_data_char;
    

    string message_type = message_type_buffer;

    if (message_type != desired_message_type)
      continue;

    // Get the corresponding header station id
    string station_id = station_id_buffer;

    if (station_id != desired_station_id)
      continue;

    // Get the corresponding header valid time


    // If we get here, this is an observation that we want to use

    SDObservation obs(hdr_vld_buffer, obs_arr[4]);

    observations.push_back(obs);
   } // end for i

  // Cleanup

  if (obs_arr) { delete [] obs_arr; obs_arr = (float *) 0; }
  if (hdr_arr) { delete [] hdr_arr; hdr_arr = (float *) 0; }

  return true;
}


////////////////////////////////////////////////////////////////////////
// Protected/Private Methods
////////////////////////////////////////////////////////////////////////

