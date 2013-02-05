

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

#include "met_handler.h"


static const int   n_met_col     = 10;
static const int   n_met_col_qty = 11;

////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetHandler
   //


////////////////////////////////////////////////////////////////////////


MetHandler::MetHandler(const string &program_name) :
  FileHandler(program_name),
  _nFileColumns(0)
{
}


////////////////////////////////////////////////////////////////////////


MetHandler::~MetHandler()
{
}


////////////////////////////////////////////////////////////////////////


bool MetHandler::isFileType(LineDataFile &ascii_file) const
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
  // Check for expected number of MET columns
  //

  if (dl.n_items() == n_met_col || dl.n_items() == n_met_col_qty)
    is_file_type = true;
   
  return is_file_type;
}
  

////////////////////////////////////////////////////////////////////////

bool MetHandler::_prepareHeaders(LineDataFile &ascii_file) {
   DataLine dl;
   ConcatString hdr_typ, hdr_sid, hdr_vld;
   double hdr_lat, hdr_lon, hdr_elv;

   mlog << Debug(1) << "Reading MET ASCII Observation file: "
        << _asciiFilename << "\n";

   //
   // Process each line of the file
   //
   while(ascii_file >> dl) {

      //
      // Store/check the number of columns in the first line
      //
      if(dl.line_number() == 1) {

         //
         // Make sure the number of columns is an acceptible format
         //
         if(dl.n_items() != n_met_col && dl.n_items() != n_met_col_qty) {
            mlog << Error << "\nMetHandler::_prepareHeaders() -> "
                 << "line number " << dl.line_number()
                 << " does not contain the expected number of columns ("
                 << n_met_col << " or " << n_met_col_qty << ").\n\n";
            return false;
         }

         //
         // Store the column format
         //
         _nFileColumns = dl.n_items();
         if(_nFileColumns == n_met_col) {
            mlog << Debug(1) << "Found 10 column input file format deprecated "
                 << "in METv4.1 - consider adding quality flag value\n";
         }

         //
         // Increment the header count and store the first line
         //
         _nhdr++;
         hdr_typ =      dl[0];
         hdr_sid =      dl[1];
         hdr_vld =      dl[2];
         hdr_lat = atof(dl[3]);
         hdr_lon = atof(dl[4]);
         hdr_elv = atof(dl[5]);

      }
      //
      // Check all following lines
      //
      else {

         //
         // Verify format consistency
         //
         if(dl.n_items() != _nFileColumns) {
            mlog << Error << "\nMetHandler::_prepareHeaders() -> "
                 << "line number " << dl.line_number()
                 << " does not have the same number of columns as the "
                 << "first line (" << _nFileColumns << ").\n\n";
            return false;
         }

         //
         // Check to see if the header info has changed
         //
         if(hdr_typ           != dl[0]   ||
            hdr_sid           != dl[1]   ||
            hdr_vld           != dl[2]   ||
            !is_eq(hdr_lat, atof(dl[3])) ||
            !is_eq(hdr_lon, atof(dl[4])) ||
            !is_eq(hdr_elv, atof(dl[5]))) {

            //
            // Increment the header count and store the current values
            //
            _nhdr++;
            hdr_typ =      dl[0];
            hdr_sid =      dl[1];
            hdr_vld =      dl[2];
            hdr_lat = atof(dl[3]);
            hdr_lon = atof(dl[4]);
            hdr_elv = atof(dl[5]);
         }
      }
   } // end while

   return true;
}

////////////////////////////////////////////////////////////////////////

bool MetHandler::_processObs(LineDataFile &ascii_file,
			     const string &nc_filename) {
   DataLine dl;
   int obs_idx;
   ConcatString hdr_typ, hdr_sid, hdr_vld, obs_qty;
   double hdr_lat, hdr_lon, hdr_elv, obs_prs, obs_hgt, obs_val;

   mlog << Debug(2) << "Processing observations for " << _nhdr
        << " headers.\n";

   //
   // Process each line of the file
   //
   while(ascii_file >> dl) {

      //
      // Check for the first line of the file or the header changing
      //
      if(dl.line_number()  == 1       ||
         hdr_typ           != dl[0]   ||
         hdr_sid           != dl[1]   ||
         hdr_vld           != dl[2]   ||
         !is_eq(hdr_lat, atof(dl[3])) ||
         !is_eq(hdr_lon, atof(dl[4])) ||
         !is_eq(hdr_elv, atof(dl[5]))) {

         //
         // Store the header info
         //
         hdr_typ =      dl[0];
         hdr_sid =      dl[1];
         hdr_vld =      dl[2];
         hdr_lat = atof(dl[3]);
         hdr_lon = atof(dl[4]);
         hdr_elv = atof(dl[5]);

         //
         // Write the header info
         //
         if (!_writeHdrInfo(hdr_typ, hdr_sid, hdr_vld,
			    hdr_lat, hdr_lon, hdr_elv))
	   return false;
      }

      //
      // Pressure level (hPa) or precip accumulation interval (sec)
      //
      obs_prs = (is_precip_grib_code(atoi(dl[6])) ?
                 timestring_to_sec(dl[7]) : atof(dl[7]));

      //
      // Observation height (meters above sea level)
      //
      obs_hgt = atof(dl[8]);

      //
      // Observation quality
      //
      obs_qty = (_nFileColumns == n_met_col ? "NA" : dl[9]);

      //
      // Observation value
      //
      obs_idx = (_nFileColumns == n_met_col ? 9 : 10);
      obs_val = atof(dl[obs_idx]);
             
      //
      // Write the observation info
      //
      if (!_writeObsInfo(atoi(dl[6]),
			 obs_prs, obs_hgt, obs_val, obs_qty))
	return false;
                        
   } // end while

   return true;
}

