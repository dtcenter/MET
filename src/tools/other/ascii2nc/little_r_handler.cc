

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

static const char *lr_rpt_reg_exp = "FM-[0-9]";
static const char *lr_rpt_sfc_str = "FM-12";
static const char *lr_rpt_upa_str = "FM-35";

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

bool LittleRHandler::_prepareHeaders(LineDataFile &ascii_file) {
   DataLine dl;

   mlog << Debug(1) << "Reading little_r ASCII Observation file: "
       << _asciiFilename << "\n";

   //
   // Read the fixed-width lines:
   //   - one header report line
   //   - variable number of data lines
   //   - one end of report line
   //
   while(ascii_file.read_fwf_line(dl, lr_rpt_wdth, n_lr_rpt_wdth)) {

      //
      // Check for expected header line
      //
      if(!check_reg_exp(lr_rpt_reg_exp, dl[4])) {
         mlog << Error << "\nLittleRHandler::_prepareHeaders() -> "
              << "the fifth entry of the little_r report on line "
              << dl.line_number() << " does not match \""
              << lr_rpt_reg_exp << "\":\n\"" << dl[4] << "\"\n\n";
         return false;
      }

      //
      // Increment the header count
      //
      _nhdr++;

      //
      // Read the data lines
      //
      while(ascii_file.read_fwf_line(dl, lr_meas_wdth, n_lr_meas_wdth)) {

         //
         // Check for the end of report
         //
         if(is_eq(atof(dl[0]), lr_end_value) &&
            is_eq(atof(dl[2]), lr_end_value)) break;
      }

      //
      // Read the end of report line
      //
      ascii_file.read_fwf_line(dl, lr_end_wdth, n_lr_end_wdth);
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

bool LittleRHandler::_processObs(LineDataFile &ascii_file,
				 const string &nc_filename) {
   DataLine dl;
   int i, i_data, n_data_hdr;
   ConcatString cs, hdr_typ, hdr_sid, hdr_vld, obs_qty;
   double hdr_elv, obs_prs, obs_hgt, obs_val;

   mlog << Debug(2) << "Processing " << _nhdr << " Little_r reports.\n";

   //
   // Read the fixed-width lines:
   //   - one header report line
   //   - variable number of data lines
   //   - one end of report line
   //
   while(ascii_file.read_fwf_line(dl, lr_rpt_wdth, n_lr_rpt_wdth)) {

      //
      // Check for expected header line
      //
      if(!check_reg_exp(lr_rpt_reg_exp, dl[4])) {
         mlog << Error << "\nLittleRHandler::_processObs() -> "
              << "the fifth entry of the little_r report on line "
              << dl.line_number() << " does not match \""
              << lr_rpt_reg_exp << "\":\n\"" << dl[4] << "\"\n\n";
         return false;
      }

      //
      // Store the message type, checking for special strings
      //
      if(strstr(dl[4], lr_rpt_sfc_str) != NULL) {
         hdr_typ = "ADPSFC";
      }
      else if(strstr(dl[4], lr_rpt_upa_str) != NULL) {
         hdr_typ = "ADPUPA";
      }
      else {
         hdr_typ = dl[4];
         hdr_typ.ws_strip();
         hdr_typ.replace(" ", "_");
      }

      //
      // Store the station id
      //
      hdr_sid = dl[2];
      hdr_sid.ws_strip();
      hdr_sid.replace(" ", "_");

      //
      // Store the valid time in YYYYMMDD_HHMMSS format
      //
      cs = dl[17];      
      cs.ws_strip();
      hdr_vld << cs_erase;
      hdr_vld.format("%.8s_%.6s", cs.text(), cs.text()+8);

      //
      // Store the elevation
      //
      hdr_elv = (is_eq(atof(dl[6]), lr_missing_value) ?
                 bad_data_float : atof(dl[6]));

      //
      // Write the header info
      //
      if (!_writeHdrInfo(hdr_typ, hdr_sid, hdr_vld,
			 atof(dl[0]), atof(dl[1]), hdr_elv))
	return false;
      
      
      //
      // Store the number of data lines specified in the header
      //
      n_data_hdr = atoi(dl[7]);

      //
      // Observation of sea level pressure in pascals.
      //
      if(!is_eq(atof(dl[18]), lr_missing_value)) {
         obs_qty = (is_eq(atof(dl[19]), lr_missing_value) ?
                    na_str : dl[19]);
         obs_qty.ws_strip();
         if (!_writeObsInfo(2, bad_data_float, hdr_elv,
			    atof(dl[18]), obs_qty))
	   return false;
      }

      //
      // Read the data lines
      //
      i_data = 0;
      while(ascii_file.read_fwf_line(dl, lr_meas_wdth, n_lr_meas_wdth)) {

         //
         // Check for the end of report
         //
         if(is_eq(atof(dl[0]), lr_end_value) &&
            is_eq(atof(dl[2]), lr_end_value)) break;

         //
         // Retrieve pressure and height
         //
         obs_prs = (is_eq(atof(dl[0]), lr_missing_value) ?
                    bad_data_float : atof(dl[0]));
         obs_hgt = (is_eq(atof(dl[2]), lr_missing_value) ?
                    bad_data_float : atof(dl[2]));

         //
         // Pressure in Little_R is stored in pascals.  Convert to
         // hectopascals for the header entry.
         //
         if(!is_bad_data(obs_prs)) obs_prs /= 100;

         //
         // Store observations of:
         //    pressure, height, temperature, dew point,
         //    wind speed, wind direction, u-wind, v-wind,
         //    relative humidity
         //
         for(i=0; i<dl.n_items(); i++) {

            //
            // Only store valid observation values with a
            // valid corresponding GRIB code
            //
            if(!is_eq(atof(dl[i]), lr_missing_value) &&
               !is_bad_data(lr_grib_codes[i/2])) {

               //
               // Observation quality
               //
               obs_qty = (is_eq(atof(dl[i+1]), lr_missing_value) ?
                          na_str : dl[i+1]);
               obs_qty.ws_strip();

               //
               // Observation value
               //
               obs_val = atof(dl[i]);
            
               //
               // Write the observation info
               //
               if (!_writeObsInfo(lr_grib_codes[i/2],
				  obs_prs, obs_hgt, obs_val, obs_qty))
		 return false;
            }

            //
            // Increment i to skip over the QC entry
            //
            i++;
         }

         //
         // Increment the data line count
         //
         i_data++;
      }

      //
      // Check that the number of data lines specified in the header
      // matches the number found
      //
      if(n_data_hdr != i_data) {
         mlog << Warning << "\nprocess_little_r_obs() -> "
              << "the number of data lines specified in the header ("
              << n_data_hdr
              << ") does not match the number found in the data ("
              << i_data << ") on line number " << dl.line_number() << ".\n\n";
      }

      //
      // Read the end of report line
      //
      ascii_file.read_fwf_line(dl, lr_end_wdth, n_lr_end_wdth);

   } // end while

   return true;
}

