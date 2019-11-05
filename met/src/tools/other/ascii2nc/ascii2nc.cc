// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   ascii2nc.cc
//
//   Description:
//      Parse ASCII observations and convert them to NetCDF.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    01-22-08  Halley Gotway  New
//   001    09-16-08  Halley Gotway  Keep track of the header values and
//                    only write out a header record when they change.
//   002    07-15-10  Halley Gotway  Store accumulation intervals in
//                    seconds rather than hours.
//   003    01-06-12  Holmes         Added use of command line class to
//                                   parse the command line arguments.
//   005    08-01-12  Oldenburg      Added support for obs quality flag.
//   006    09-13-12  Halley Gotway  Added support for Little_r and
//                    factored out common code.
//   007    02-06-13  Rehak          Added support for surfrad data.
//   008    03-13-13  Rehak          Added optional summarization of obs.
//   009    03-26-13  Rehak          Updated configuration file
//                                     specification, changed how the
//                                     summary width is specified in the
//                                     netCDF file and added summary info
//                                     to the netCDF global attributes.
//   010    05-21-14  Halley Gotway  Print usage for fewer than 2 files
//                                     on the command line.
//   011    07-07-14  Halley Gotway  Added the mask_grid and mask_poly
//                                     options to filter spatially.
//   012    07-23-14  Halley Gotway  Add message_type_map configuration
//                                     file option.
//   013    09-21-15  Prestopnik     Add Aeronet observations.
//   014    07-23-18  Halley Gotway  Support masks defined by gen_vx_mask.
//   015    03-20-19  Fillmore       Add aeronetv2 and aeronetv3 options.
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ctype.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <regex.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <netcdf>
using namespace netCDF;

#include "data2d_factory.h"
#include "mask_poly.h"
#include "apply_mask.h"
#include "vx_grid.h"
#include "vx_nc_util.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_log.h"

#include "ascii2nc_conf_info.h"
#include "file_handler.h"
#include "little_r_handler.h"
#include "met_handler.h"
#include "surfrad_handler.h"
#include "wwsis_handler.h"
#include "aeronet_handler.h"

////////////////////////////////////////////////////////////////////////

// Constants
static const char *program_name = "ascii2nc";

static const char *DEFAULT_CONFIG_FILENAME =
  "MET_BASE/config/Ascii2NcConfig_default";

////////////////////////////////////////////////////////////////////////

// Supported input ASCII formats
enum ASCIIFormat {
   ASCIIFormat_None,
   ASCIIFormat_MET,
   ASCIIFormat_Little_R,
   ASCIIFormat_SurfRad,
   ASCIIFormat_WWSIS,
   ASCIIFormat_Aeronet_v2,
   ASCIIFormat_Aeronet_v3
};
static ASCIIFormat ascii_format = ASCIIFormat_None;

////////////////////////////////////////////////////////////////////////

// Variables for command line arguments
static vector< ConcatString > asfile_list;
static ConcatString ncfile;

static ConcatString config_filename(replace_path(DEFAULT_CONFIG_FILENAME));
static Ascii2NcConfInfo config_info;

static Grid        mask_grid;
static MaskPlane   mask_area;
static MaskPoly    mask_poly;
static StringArray mask_sid;

static int compress_level = -1;

////////////////////////////////////////////////////////////////////////

static FileHandler *create_file_handler(const ASCIIFormat,
                                        const ConcatString &);
static FileHandler *determine_ascii_format(const ConcatString &);

static void usage();
static void set_format(const StringArray &);
static void set_logfile(const StringArray &);
static void set_config(const StringArray &);
static void set_mask_grid(const StringArray &);
static void set_mask_poly(const StringArray &);
static void set_mask_sid(const StringArray &);
static void set_verbosity(const StringArray &);
static void set_compress(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   CommandLine cline;

   //
   // Set handler to be called for memory allocation error
   //
   set_new_handler(oom);

   //
   // Check for zero arguments
   //
   if(argc == 1) usage();

   //
   // Parse the command line into tokens
   //
   cline.set(argc, argv);

   //
   // Set the usage function
   //
   cline.set_usage(usage);

   //
   // Add the options function calls
   //
   cline.add(set_format,    "-format",    1);
   cline.add(set_logfile,   "-log",       1);
   cline.add(set_verbosity, "-v",         1);
   cline.add(set_config,    "-config",    1);
   cline.add(set_mask_grid, "-mask_grid", 1);
   cline.add(set_mask_poly, "-mask_poly", 1);
   cline.add(set_mask_sid,  "-mask_sid",  1);
   cline.add(set_compress,  "-compress",  1);

   //
   // Parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be at least two arguments left:
   // the ascii input filenames and the netCDF output filename
   //
   if(cline.n() < 2) usage();

   //
   // Store the input ASCII file name and the output NetCDF file name
   //
   for (int i = 0; i < cline.n() - 1; ++i)
     asfile_list.push_back((string)cline[i]);
   ncfile = cline[cline.n() - 1];

   //
   // Read the config file
   //
   mlog << Debug(1)
        << "Config File: " << config_filename << "\n";
   config_info.read_config(DEFAULT_CONFIG_FILENAME, config_filename.text());

   //
   // Create the file handler based on the ascii format specified on
   // the command line.  If one wasn't specified, we'll look in the
   // first file to guess the format.
   //
   FileHandler *file_handler = create_file_handler(ascii_format,
                                                   asfile_list[0]);

   if(file_handler == 0) return(0);

   int deflate_level = compress_level;
   if(deflate_level < 0) deflate_level = config_info.get_compression_level();
   if(deflate_level > 9) deflate_level = config_info.get_compression_level();
   file_handler->setCompressionLevel(deflate_level);
   file_handler->setSummaryInfo(config_info.getSummaryInfo());

   //
   // Set the masking grid and polyline, if specified.
   //
   if(mask_grid.nx() > 0 || mask_grid.ny() > 0) file_handler->setGridMask(mask_grid);
   if(!mask_area.is_empty())                    file_handler->setAreaMask(mask_area);
   if(mask_poly.n_points() > 0)                 file_handler->setPolyMask(mask_poly);
   if(mask_sid.n_elements() > 0)                file_handler->setSIDMask(mask_sid);

   //
   // Load the message type map
   //
   file_handler->setMessageTypeMap(config_info.getMessageTypeMap());

   //
   // Process the files.  If a configuration file was specified, do any
   // extra processing specified.
   //
   file_handler->readAsciiFiles(asfile_list);

   //
   // Summarize the observations, if directed.  We need to use a different
   // call to writeNetcdfFile in this case so that we can include the
   // summarization details.
   //
   if(config_info.getSummaryInfo().flag) {
      file_handler->summarizeObs(config_info.getSummaryInfo());
   }

   int status = file_handler->writeNetcdfFile(ncfile.text());
   delete file_handler;

   if(!status) return(1);

   return(0);
}

////////////////////////////////////////////////////////////////////////

FileHandler *create_file_handler(const ASCIIFormat format,
                                 const ConcatString &ascii_filename) {
   //
   // If the ASCII format was specified, just create the appropriate
   // object and return it.  If it wasn't specified, look in the
   // file to guess the format.
   //
   switch(format) {
      case ASCIIFormat_MET: {
         return((FileHandler *) new MetHandler(program_name));
      }

      case ASCIIFormat_Little_R: {
         return((FileHandler *) new LittleRHandler(program_name));
      }

      case ASCIIFormat_SurfRad: {
         return((FileHandler *) new SurfradHandler(program_name));
      }

      case ASCIIFormat_WWSIS: {
         return((FileHandler *) new WwsisHandler(program_name));
      }

      case ASCIIFormat_Aeronet_v2: {
         AeronetHandler *handler = new AeronetHandler(program_name);
         handler->setFormatVersion(2);
         return((FileHandler *) handler);
      }

      case ASCIIFormat_Aeronet_v3: {
         AeronetHandler *handler = new AeronetHandler(program_name);
         handler->setFormatVersion(3);
         return((FileHandler *) handler);
      }

      default: {
        return(determine_ascii_format(ascii_filename));
      }
   }

   return(0);
}

////////////////////////////////////////////////////////////////////////

FileHandler *determine_ascii_format(const ConcatString &ascii_filename) {

   //
   // Use the contents of the file to try to guess its format.
   //

   //
   // Open the input ASCII observation file
   //
   LineDataFile f_in;

   if(!f_in.open(ascii_filename.c_str())) {
     mlog << Error << "\ndetermine_ascii_format() -> "
          << "can't open input ASCII file \"" << ascii_filename
          << "\" for reading\n\n";
     exit(1);
   }

   //
   // See if this is a MET file.
   //
   f_in.rewind();
   MetHandler *met_file = new MetHandler(program_name);

   if (met_file->isFileType(f_in)) {
     f_in.close();
     return((FileHandler *) met_file);
   }

   delete met_file;

   //
   // See if this is a Little R file.
   //
   f_in.rewind();
   LittleRHandler *little_r_file = new LittleRHandler(program_name);

   if (little_r_file->isFileType(f_in)) {
     f_in.close();
     return((FileHandler *) little_r_file);
   }

   delete little_r_file;

   //
   // See if this is a SURFRAD file.
   //
   f_in.rewind();
   SurfradHandler *surfrad_file = new SurfradHandler(program_name);

   if (surfrad_file->isFileType(f_in)) {
     f_in.close();
     return((FileHandler *) surfrad_file);
   }

   delete surfrad_file;

   //
   // See if this is a WWSIS file.
   //
   f_in.rewind();
   WwsisHandler *wwsis_file = new WwsisHandler(program_name);

   if(wwsis_file->isFileType(f_in)) {
     f_in.close();
     return((FileHandler *) wwsis_file);
   }

   delete wwsis_file;

   //
   // See if this is a Aeronet file.
   //
   f_in.rewind();
   AeronetHandler *aeronet_file = new AeronetHandler(program_name);

   if(aeronet_file->isFileType(f_in)) {
     f_in.close();
     return((FileHandler *) aeronet_file);
   }

   delete aeronet_file;

   //
   // If we get here, we didn't recognize the file contents.
   //

   mlog << Error << "\ndetermine_ascii_format() -> "
        << "could not determine file format based on file contents\n\n";

   f_in.close();

   exit(1);

   return(0);
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\nUsage: "
        << program_name << "\n"
        << "\tascii_file1 [ascii_file2 ... ascii_filen]\n"
        << "\tnetcdf_file\n"
        << "\t[-format ASCII_format]\n"
        << "\t[-config file]\n"
        << "\t[-mask_grid string]\n"
        << "\t[-mask_poly file]\n"
        << "\t[-mask_sid file|list]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"ascii_file\" is the formatted ASCII "
        << "observation file to be converted to NetCDF format "
        << "(required).\n"

        << "\t\t\"netcdf_file\" indicates the name of the output "
        << "NetCDF file to be written (required).\n"

        << "\t\t\"-format ASCII_format\" may be set to \""
        << MetHandler::getFormatString() << "\", \""
        << LittleRHandler::getFormatString() << "\", \""
        << SurfradHandler::getFormatString() << "\", \""
        << WwsisHandler::getFormatString() << "\", \""
        << AeronetHandler::getFormatString() << "\", \""
        << AeronetHandler::getFormatString_v2() << "\", or \""
        << AeronetHandler::getFormatString_v3() << "\" (optional).\n"

        << "\t\t\"-config file\" uses the specified configuration file "
        << "to generate summaries of the fields in the ASCII files (optional).\n"

        << "\t\t\"-mask_grid string\" is a named grid or a data file defining "
        << "the grid for filtering the point observations spatially (optional).\n"

        << "\t\t\"-mask_poly file\" is a polyline file, the output of gen_vx_mask, "
        << "or a gridded data file with field information for filtering "
        << "the point observations spatially (optional).\n"

        << "\t\t\"-mask_sid file|list\" is a station ID masking file or a "
        << "comma-separated list of station ID's for filtering the point "
        << "observations spatially (optional).\n"
        << "\t\t   For a list of length greater than one, the first element is "
        << "stored as the mask name (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n"

        << "\t\t\"-compress level\" overrides the compression level of NetCDF variable ("
        << config_info.get_compression_level() << ") (optional).\n\n"

        << "\tThe \"" << MetHandler::getFormatString()
        << "\" ASCII format consists of 11 columns:\n"
        << "\t\tMessage_Type Station_ID Valid_Time(YYYYMMDD_HHMMSS)\n"
        << "\t\tLat(Deg North) Lon(Deg East) Elevation(msl)\n"
        << "\t\tVar_Name(or GRIB_Code) Level Height(msl or agl)\n"
        << "\t\tQC_String Observation_Value\n\n"

        << "\t\twhere\t\"Level\" is the pressure level (hPa) or "
        << "accumulation interval (HH[MMSS]).\n"
        << "\t\t\t\"Height\" is meters above sea level or above ground level for the "
        << "observation (msl or agl).\n\n"

        << "\t\t\tUse a value of \"" << bad_data_int
        << "\" or \"" << na_str << "\" to indicate missing data.\n\n"

        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_format(const StringArray & a) {

   if(MetHandler::getFormatString() == a[0]) {
      ascii_format = ASCIIFormat_MET;
   }
   else if(LittleRHandler::getFormatString() == a[0]) {
     ascii_format = ASCIIFormat_Little_R;
   }
   else if(SurfradHandler::getFormatString() == a[0]) {
     ascii_format = ASCIIFormat_SurfRad;
   }
   else if(WwsisHandler::getFormatString() == a[0]) {
     ascii_format = ASCIIFormat_WWSIS;
   }
   else if(AeronetHandler::getFormatString() == a[0]
     || AeronetHandler::getFormatString_v2() == a[0]) {
     ascii_format = ASCIIFormat_Aeronet_v2;
   }
   else if(AeronetHandler::getFormatString_v3() == a[0]) {
     ascii_format = ASCIIFormat_Aeronet_v3;
   }
   else {
      mlog << Error << "\nset_format() -> "
           << "unsupported ASCII observation format \""
           << a[0] << "\".\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_config(const StringArray & a) {
   config_filename = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_mask_grid(const StringArray & a) {

   // List the grid masking file
   mlog << Debug(1)
        << "Grid Masking: " << a[0] << "\n";

   parse_grid_mask(a[0], mask_grid);

   // List the grid mask
   mlog << Debug(2)
        << "Parsed Masking Grid: " << mask_grid.name() << " ("
        << mask_grid.nx() << " x " << mask_grid.ny() << ")\n";
}

////////////////////////////////////////////////////////////////////////

void set_mask_poly(const StringArray & a) {
   ConcatString mask_name;

   // List the poly masking file
   mlog << Debug(1)
        << "Polyline Masking File: " << a[0] << "\n";

   parse_poly_mask(a[0], mask_poly, mask_grid, mask_area, mask_name);

   // List the mask information
   if(mask_poly.n_points() > 0) {
      mlog << Debug(2)
           << "Parsed Masking Polyline: " << mask_poly.name()
           << " containing " <<  mask_poly.n_points() << " points\n";
   }

   // List the area mask information
   if(mask_area.nx() > 0 || mask_area.ny() > 0) {
      mlog << Debug(2)
           << "Parsed Masking Area: " << mask_name
           << " for (" << mask_grid.nx() << " x " << mask_grid.ny()
           << ") grid\n";
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void set_mask_sid(const StringArray & a) {
   ConcatString mask_name;

   // List the station ID mask
   mlog << Debug(1)
        << "Station ID Mask: " << a[0] << "\n";

   parse_sid_mask(a[0], mask_sid, mask_name);

   // List the length of the station ID mask
   mlog << Debug(2)
        << "Parsed Station ID Mask: " << mask_name
        << " containing " << mask_sid.n_elements() << " points\n";
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

