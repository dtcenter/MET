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
#include <map>

#include "vx_log.h"
#include "vx_math.h"

#include "little_r_handler.h"


static const double lr_end_value     = -777777.0;
static const double lr_missing_value = -888888.0;

static const int lr_rpt_wdth[] = {
   20, 20, 40, 40, 40, 40, 20,
   10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 20,
   13,  7, 13,  7, 13,  7, 13,  7, 13,  7,
   13,  7, 13,  7, 13,  7, 13,  7, 13,  7,
   13,  7, 13,  7, 13,  7
};
static int n_lr_rpt_wdth = sizeof(lr_rpt_wdth)/sizeof(*lr_rpt_wdth);

// Little-R fixed widths for the data lines
static const int lr_meas_wdth[] = {
   13,  7, 13,  7, 13,  7, 13,  7, 13,  7,
   13,  7, 13,  7, 13,  7, 13,  7, 13,  7
};   
static int n_lr_meas_wdth = sizeof(lr_meas_wdth)/sizeof(*lr_meas_wdth);

// Little-R fixed widths for the end of report lines
static const int lr_end_wdth[] = {
    7,  7,  7
};
static int n_lr_end_wdth = sizeof(lr_end_wdth)/sizeof(*lr_end_wdth);

// GRIB codes for entries in the Little-R data lines
static const int lr_grib_codes[] = {
   1, 7, 11, 17, 32, 31, 33, 34, 52, bad_data_int
};
static const string lr_grib_names[] = {
   "PRES",      // 001	PRES	Pressure	Pa
   "HGT",       // 007	HGT	Geopotential height	gpm
   "TMP",       // 011	TMP	Temperature	K
   "DPT",       // 017	DPT	Dewpoint temperature	K
   "WIND",      // 032	WIND	Wind speed	m s-1
   "WDIR",      // 031	WDIR	Wind direction	deg
   "UGRD",      // 033	U GRD	u-component of wind	m s-1
   "VGRD",      // 034	V GRD	v-component of wind	m s-1
   "RH",        // 052	R H	Relative humidity	% 
   "UNKNOWN"    // bad_data_int
};

// Little-R regular expression used to determine file type
static const char *lr_rpt_reg_exp = "FM-[0-9]";

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class LittleRHandler
   //


////////////////////////////////////////////////////////////////////////


LittleRHandler::LittleRHandler(const string &program_name) :
  FileHandler(program_name)
{
}


////////////////////////////////////////////////////////////////////////


LittleRHandler::~LittleRHandler()
{
}


////////////////////////////////////////////////////////////////////////


bool LittleRHandler::isFileType(LineDataFile &ascii_file) const
{
  //
  // Initialize the return value.
  //
  bool is_file_type = false;
  
  //
  // Read the first line from the file
  //
  DataLine dl;
  ascii_file >> dl;

  //
  // Check for a Little_R regular expression
  //
  for (int i = 0; i<dl.n_items(); i++) {
    if(check_reg_exp(lr_rpt_reg_exp, dl[i])) {
      is_file_type = true;
      break;
    }
   }

  return is_file_type;
}
  

////////////////////////////////////////////////////////////////////////
// Protected/Private Methods
////////////////////////////////////////////////////////////////////////

bool LittleRHandler::_readObservations(LineDataFile &ascii_file)
{
  // Read the fixed-width lines:
  //   - one header report line
  //   - variable number of data lines
  //   - one end of report line

  DataLine data_line;
  int n_data_hdr;
  StringArray mappedTypes;
  StringArray unmappedTypes;
  
  while (ascii_file.read_fwf_line(data_line, lr_rpt_wdth, n_lr_rpt_wdth))
  {
    // Check for expected header line

    if (!check_reg_exp(lr_rpt_reg_exp, data_line[4]))
    {
      mlog << Error << "\nLittleRHandler::_readObservations() -> "
           << "the fifth entry of the little_r report on line "
           << data_line.line_number() << " does not match \""
           << lr_rpt_reg_exp << "\":\n\"" << data_line[4] << "\"\n\n";
      return false;
    }

    // Store the message type

    ConcatString concat_string = (string)data_line[4];
    concat_string.ws_strip();
    ConcatString hdr_typ;
    
    if (_messageTypeMap[concat_string] != "")
    {
      hdr_typ = _messageTypeMap[concat_string];
      if (!mappedTypes.has(concat_string)) {
         mlog << Debug(5)
              << "Switching little_r report type \"" << concat_string
              << "\" to message type \"" << hdr_typ << "\".\n";
         mappedTypes.add(concat_string);
      }
    }
    else
    {
      hdr_typ = concat_string;
      hdr_typ.replace(" ", "_", false);
      
      if (!unmappedTypes.has(concat_string)) {
         mlog << Warning << "\nLittleRHandler::_processObs() -> "
              << "Storing message type as \"" << hdr_typ
              << "\" for unexpected report type \"" << concat_string << "\".\n\n";
         unmappedTypes.add(concat_string);
      }
    }

    // Store the station id

    ConcatString hdr_sid = (string)data_line[2];
    hdr_sid.ws_strip();
    hdr_sid.replace(" ", "_", false);

    // Store the valid time in YYYYMMDD_HHMMSS format

    ConcatString hdr_vld_str;
    
    concat_string = data_line[17];      
    concat_string.ws_strip();
    hdr_vld_str << cs_erase;
    hdr_vld_str.format("%.8s_%.6s",
                       concat_string.text(), concat_string.text()+8);

    time_t hdr_vld = _getValidTime(hdr_vld_str.text());
    
    // Store the station location

    double hdr_lat = atof(data_line[0]);
    double hdr_lon = atof(data_line[1]);
    double hdr_elv = (is_eq(atof(data_line[6]), lr_missing_value) ?
                      bad_data_float : atof(data_line[6]));

    // Store the number of data lines specified in the header

    n_data_hdr = atoi(data_line[7]);

    // Observation of sea level pressure in pascals.

    if (!is_eq(atof(data_line[18]), lr_missing_value))
    {
      ConcatString obs_qty = (is_eq(atof(data_line[19]), lr_missing_value) ?
                              na_string : (string)data_line[19]);
      obs_qty.ws_strip();

      // 002	PRMSL	Pressure reduced to MSL	Pa
      _addObservations(Observation(hdr_typ.text(),
                                   hdr_sid.text(),
                                   hdr_vld,
                                   hdr_lat,
                                   hdr_lon,
                                   hdr_elv,
                                   obs_qty.text(),
                                   2,
                                   bad_data_float,
                                   hdr_elv,
                                   atof(data_line[18]),
                                   "PRMSL"));
    }

    // Read the data lines

    int i_data = 0;
    while (ascii_file.read_fwf_line(data_line, lr_meas_wdth, n_lr_meas_wdth))
    {
      // Check for the end of report

      if (is_eq(atof(data_line[0]), lr_end_value) &&
              is_eq(atof(data_line[2]), lr_end_value))
        break;

      // Retrieve pressure and height

      double obs_prs = (is_eq(atof(data_line[0]), lr_missing_value) ?
                        bad_data_float : atof(data_line[0]));
      double obs_hgt = (is_eq(atof(data_line[2]), lr_missing_value) ?
                        bad_data_float : atof(data_line[2]));

      // Pressure in Little_R is stored in pascals.  Convert to
      // hectopascals for the header entry.

      if (!is_bad_data(obs_prs)) obs_prs /= 100;

      // Store observations of:
      //    pressure, height, temperature, dew point,
      //    wind speed, wind direction, u-wind, v-wind,
      //    relative humidity

      for (int i = 0; i < data_line.n_items(); ++i)
      {
        // Only store valid observation values with a
        // valid corresponding GRIB code

        if (!is_eq(atof(data_line[i]), lr_missing_value) &&
            !is_bad_data(lr_grib_codes[i/2]))
        {
          // Observation quality

          ConcatString obs_qty =
            (is_eq(atof(data_line[i+1]), lr_missing_value) ?
             na_string : (string)data_line[i+1]);
          obs_qty.ws_strip();

          // Observation value

          double obs_val = atof(data_line[i]);
            
          // Write the observation info

          _addObservations(Observation(hdr_typ.text(), hdr_sid.text(),
                                       hdr_vld,
                                       hdr_lat, hdr_lon, hdr_elv,
                                       obs_qty.text(),
                                       lr_grib_codes[i/2],
                                       obs_prs, obs_hgt, obs_val,
                                       lr_grib_names[i/2]));
        }

        // Increment i to skip over the QC entry

        i++;
      }

      // Increment the data line count

      i_data++;
    }

    // Check that the number of data lines specified in the header
    // matches the number found

    if (n_data_hdr != i_data)
    {
      mlog << Warning << "\nprocess_little_r_obs() -> "
           << "the number of data lines specified in the header ("
           << n_data_hdr
           << ") does not match the number found in the data ("
           << i_data << ") on line number " << data_line.line_number() << ".\n\n";
    }

      // Read the end of report line

    ascii_file.read_fwf_line(data_line, lr_end_wdth, n_lr_end_wdth);

  } // end while

  return true;
}
  
