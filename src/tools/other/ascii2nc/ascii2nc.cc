// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
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
//   016    01-30-20  Bullock        Add python option.
//   017    01-25-21  Halley Gotway  MET #1630 Handle zero obs.
//   018    03-01-21  Fillmore       Replace pickle files for temporary
//                                   ascii.
//   019    07/06/22  Howard Soh     METplus-Internal #19 Rename main to met_main
//   020    08/26/22  Dave Albo      MET #2142 Add AirNow observations
//   021    10/03/22  Prestopnik     MET #2227 Remove using namespace std from header files
//   022    10/07/22  Dave Albo      MET #2276 Add NDBC buoy data
//   023    11/28/23  Halley Gotway  MET #2701 Add ISMN soil moisture data
//   024    01/06/25  Halley Gotway  MET #1019 Add USCRN quality controlled data
//   025    06/23/25  Halley Gotway  MET #3148 Search input directories
//
////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <dirent.h>
#include <fstream>
#include <math.h>
#include <regex.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "main.h"

#include "data2d_factory.h"
#include "mask_poly.h"
#include "apply_mask.h"
#include "parse_file_list.h"
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
#include "airnow_handler.h"
#include "ndbc_handler.h"
#include "ismn_handler.h"
#include "iabp_handler.h"
#include "uscrn_handler.h"

#ifdef ENABLE_PYTHON
#include "global_python.h"
#include "python_handler.h"
#endif

using namespace std;


////////////////////////////////////////////////////////////////////////

// Constants
static const char *program_name = "ascii2nc";
static const char *default_reg_exp = ".*";
static const char *DEFAULT_CONFIG_FILENAME =
  "MET_BASE/config/Ascii2NcConfig_default";

////////////////////////////////////////////////////////////////////////

// Supported input ASCII formats
enum class ASCIIFormat {
   None,
   MET,
   Little_R,
   SurfRad,
   WWSIS,
   Airnow_dailyv2,
   Airnow_hourlyaqobs,
   Airnow_hourly,
   NDBC_standard,
   ISMN,
   IABP,
   USCRN,
   Aeronet_v2,
   Aeronet_v3, 
   Python, 
};
static ASCIIFormat ascii_format = ASCIIFormat::None;

////////////////////////////////////////////////////////////////////////

// Variables for command line arguments
static vector<ConcatString> asfile_list;
static ConcatString ncfile;
static ConcatString input_reg_exp(default_reg_exp);

static ConcatString config_filename(replace_path(DEFAULT_CONFIG_FILENAME));
static Ascii2NcConfInfo config_info;

static Grid        mask_grid;
static MaskPlane   mask_area;
static MaskPoly    mask_poly;
static StringArray mask_sid;

// Beginning and ending times
static unixtime valid_beg_ut;
static unixtime valid_end_ut;

static int compress_level = -1;

////////////////////////////////////////////////////////////////////////

static StringArray get_input_files(const ConcatString &);
static FileHandler *create_file_handler(const ASCIIFormat,
                                        const ConcatString &);
static FileHandler *determine_ascii_format(const ConcatString &,
                                           ConcatString &);

static void usage();
static void set_inputrx(const StringArray &);
static void set_format(const StringArray &);
static void set_config(const StringArray &);
static void set_mask_grid(const StringArray &);
static void set_mask_poly(const StringArray &);
static void set_mask_sid(const StringArray &);
static void set_compress(const StringArray &);
static void set_valid_beg_time(const StringArray &);
static void set_valid_end_time(const StringArray &);

static void setup_wrapper_path();

////////////////////////////////////////////////////////////////////////

int met_main(int argc, char *argv[]) {
   CommandLine cline;

   //
   // Check for zero arguments
   //
   if(argc == 1) { usage(); return 0; }

   // Initialize time range
   valid_beg_ut = valid_end_ut = (unixtime) 0;

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
   cline.add(set_inputrx,        "-inputrx",   1);
   cline.add(set_format,         "-format",    1);
   cline.add(set_config,         "-config",    1);
   cline.add(set_mask_grid,      "-mask_grid", 1);
   cline.add(set_mask_poly,      "-mask_poly", 1);
   cline.add(set_mask_sid,       "-mask_sid",  1);
   cline.add(set_valid_beg_time, "-valid_beg", 1);
   cline.add(set_valid_end_time, "-valid_end", 1);
   cline.add(set_compress,       "-compress",  1);

   //
   // Parse the command line
   //
   cline.parse();

   //
   // Check for error. There should be at least two arguments left:
   // the ascii input filenames and the netCDF output filename
   //
   if(cline.n() < 2) { usage(); return 0; }

   //
   // Store the input ASCII file names
   //
   for(int i=0; i<cline.n()-1; i++) {
      StringArray cur_files(get_input_files(cline[i]));
      for(int j=0; j<cur_files.n(); j++) {
         asfile_list.emplace_back(cur_files[j]);
      }
   }

   //
   // Check for at least one input file
   //
   if(asfile_list.empty()) {
      mlog << Error << "\nmet_main() -> "
           << "No input files found!\n\n";
      exit(1);
   }

   //
   // Store the output NetCDF file name
   //
   ncfile = cline[cline.n() - 1];

   //
   // Read the config file
   //
   mlog << Debug(1)
        << "Config File: " << config_filename << "\n";
   config_info.read_config(DEFAULT_CONFIG_FILENAME, config_filename.text());

   // Check that valid_end_ut >= valid_beg_ut
   if(valid_beg_ut != (unixtime) 0 &&
      valid_end_ut != (unixtime) 0 &&
      valid_beg_ut > valid_end_ut) {
      mlog << Error << "\nmet_main() -> "
           << "the ending time (" << unix_to_yyyymmdd_hhmmss(valid_end_ut)
           << ") must be greater than the beginning time ("
           << unix_to_yyyymmdd_hhmmss(valid_beg_ut) << ").\n\n";
      exit(1);
   }

   //
   // Create the file handler based on the ascii format specified on
   // the command line.  If one wasn't specified, we'll look in the
   // first file to guess the format.
   //
   FileHandler *file_handler = create_file_handler(ascii_format, asfile_list[0]);

   if(file_handler == 0) return 0;

   int deflate_level = compress_level;
   if(deflate_level < 0) deflate_level = config_info.get_compression_level();
   if(deflate_level > 9) deflate_level = config_info.get_compression_level();
   file_handler->setCompressionLevel(deflate_level);
   file_handler->setSummaryInfo(config_info.getSummaryInfo());
   file_handler->setValidTimeRange(valid_beg_ut, valid_end_ut);
   
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
   // Read the input files
   //
   if(!file_handler->readAsciiFiles(asfile_list)) {
      mlog << Warning << "\n" << program_name << " -> "
           << "encountered error(s) while reading input files!\n\n";
   }

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

   if(!status) return 1;

   return 0;

}

////////////////////////////////////////////////////////////////////////

const string get_tool_name() {
   return program_name;
}

////////////////////////////////////////////////////////////////////////

static StringArray get_input_files(const ConcatString &input) {
   StringArray sa;

   // Search input directories
   if(is_directory(input.c_str())) {
      sa.add(get_filenames(input, nullptr, input_reg_exp.c_str()));
   }
   // Process ASCII file list
   else if(is_ascii_file_list(input.c_str())) {
      sa.add(parse_ascii_file_list(input.c_str()));
   }
   // Store file names and python inputs
   else {
      sa.add(input);
   }

   return sa;
}

////////////////////////////////////////////////////////////////////////

static FileHandler *create_file_handler(const ASCIIFormat format,
                                        const ConcatString &ascii_filename) {

   #ifdef ENABLE_PYTHON
   PythonHandler * ph = 0;
   #endif

   //
   // If the ASCII format was specified, just create the appropriate
   // object and return it.  If it wasn't specified, look in the
   // file to guess the format.
   //
   switch(format) {
      case ASCIIFormat::MET: {
         return (FileHandler *) new MetHandler(program_name);
      }

      case ASCIIFormat::Little_R: {
         return (FileHandler *) new LittleRHandler(program_name);
      }

      case ASCIIFormat::SurfRad: {
         return (FileHandler *) new SurfradHandler(program_name);
      }

      case ASCIIFormat::WWSIS: {
         return (FileHandler *) new WwsisHandler(program_name);
      }

      case ASCIIFormat::Airnow_dailyv2: {
         AirnowHandler *handler = new AirnowHandler(program_name);
         handler->setFormatVersion(AirnowHandler::AIRNOW_FORMAT_VERSION_DAILYV2);
         return (FileHandler *) handler;
      }

      case ASCIIFormat::Airnow_hourlyaqobs: {
         AirnowHandler *handler = new AirnowHandler(program_name);
         handler->setFormatVersion(AirnowHandler::AIRNOW_FORMAT_VERSION_HOURLYAQOBS);
         return (FileHandler *) handler;
      }

      case ASCIIFormat::Airnow_hourly: {
         AirnowHandler *handler = new AirnowHandler(program_name);
         handler->setFormatVersion(AirnowHandler::AIRNOW_FORMAT_VERSION_HOURLY);
         return (FileHandler *) handler;
      }

      case ASCIIFormat::NDBC_standard: {
         NdbcHandler *handler = new NdbcHandler(program_name);
         handler->setFormatVersion(NdbcHandler::NDBC_FORMAT_VERSION_STANDARD);
         return (FileHandler *) handler;
      }

      case ASCIIFormat::ISMN: {
         return (FileHandler *) new IsmnHandler(program_name);
      }

      case ASCIIFormat::IABP: {
         return((FileHandler *) new IabpHandler(program_name));
      }

      case ASCIIFormat::USCRN: {
         return((FileHandler *) new UscrnHandler(program_name));
      }

      case ASCIIFormat::Aeronet_v2: {
         AeronetHandler *handler = new AeronetHandler(program_name);
         handler->setFormatVersion(2);
         return (FileHandler *) handler;
      }

      case ASCIIFormat::Aeronet_v3: {
         AeronetHandler *handler = new AeronetHandler(program_name);
         handler->setFormatVersion(3);
         return (FileHandler *) handler;
      }
      #ifdef ENABLE_PYTHON
      case ASCIIFormat::Python: {
         setup_wrapper_path();
         ph = new PythonHandler(program_name);
         return (FileHandler *) ph;
      }
      #endif

      default: {
         ConcatString format_string;
         FileHandler *guess = determine_ascii_format(ascii_filename,
                                                     format_string);
         mlog << Debug(2) << "Applying \"-format " << format_string
              << "\" to read input files. Specify the \"-format\" "
              << "option to override this default setting.\n";
	 return guess;
      }
   }
}

////////////////////////////////////////////////////////////////////////

static FileHandler *determine_ascii_format(const ConcatString &ascii_filename,
                                           ConcatString &format_string) {

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
   // See if this is an IABP file.
   // put this first as it can have the same number of columns as some
   // other ones, which look only at the number of columns
   //
   f_in.rewind();
   IabpHandler *iabp_file = new IabpHandler(program_name);

   if(iabp_file->isFileType(f_in)) {
     f_in.close();
     format_string = IabpHandler::getFormatString();
     return((FileHandler *) iabp_file);
   }

   delete iabp_file;

   //
   // See if this is a MET file.
   //
   f_in.rewind();
   MetHandler *met_file = new MetHandler(program_name);

   if (met_file->isFileType(f_in)) {
     f_in.close();
     format_string = MetHandler::getFormatString();
     return (FileHandler *) met_file;
   }

   delete met_file;

   //
   // See if this is a Little R file.
   //
   f_in.rewind();
   LittleRHandler *little_r_file = new LittleRHandler(program_name);

   if (little_r_file->isFileType(f_in)) {
     f_in.close();
     format_string = LittleRHandler::getFormatString();
     return (FileHandler *) little_r_file;
   }

   delete little_r_file;

   //
   // See if this is a SURFRAD file.
   //
   f_in.rewind();
   SurfradHandler *surfrad_file = new SurfradHandler(program_name);

   if (surfrad_file->isFileType(f_in)) {
     f_in.close();
     format_string = SurfradHandler::getFormatString();
     return (FileHandler *) surfrad_file;
   }

   delete surfrad_file;

   //
   // See if this is a WWSIS file.
   //
   f_in.rewind();
   WwsisHandler *wwsis_file = new WwsisHandler(program_name);

   if(wwsis_file->isFileType(f_in)) {
     f_in.close();
     format_string = WwsisHandler::getFormatString();
     return (FileHandler *) wwsis_file;
   }

   delete wwsis_file;

   //
   // See if this is a Aeronet file.
   //
   f_in.rewind();
   AeronetHandler *aeronet_file = new AeronetHandler(program_name);

   if(aeronet_file->isFileType(f_in)) {
     f_in.close();
     format_string = AeronetHandler::getFormatString();
     return (FileHandler *) aeronet_file;
   }

   delete aeronet_file;

   //
   // See if this is an Airnow file.
   //
   f_in.rewind();
   AirnowHandler *airnow_file = new AirnowHandler(program_name);

   if(airnow_file->isFileType(f_in)) {
     f_in.close();
     format_string = AirnowHandler::getFormatStringDailyV2();
     return (FileHandler *) airnow_file;
   }

   delete airnow_file;

   //
   // See if this is an NDBC file.
   //
   f_in.rewind();
   NdbcHandler *ndbc_file = new NdbcHandler(program_name);

   if(ndbc_file->isFileType(f_in)) {
     f_in.close();
     format_string = NdbcHandler::getFormatStringStandard();
     return (FileHandler *) ndbc_file;
   }

   delete ndbc_file;

   //
   // See if this is an ISMN file.
   //
   f_in.rewind();
   IsmnHandler *ismn_file = new IsmnHandler(program_name);

   if(ismn_file->isFileType(f_in)) {
     f_in.close();
     format_string = IsmnHandler::getFormatString();
     return (FileHandler *) ismn_file;
   }

   delete ismn_file;

   //
   // See if this is a USCRN file.
   //
   f_in.rewind();
   UscrnHandler *uscrn_file = new UscrnHandler(program_name);

   if(uscrn_file->isFileType(f_in)) {
     f_in.close();
     format_string = UscrnHandler::getFormatString();
     return (FileHandler *) uscrn_file;
   }

   delete uscrn_file;

   //
   // If we get here, we didn't recognize the file contents.
   //
   mlog << Error << "\ndetermine_ascii_format() -> "
        << "could not determine file format based on file contents\n\n";

   f_in.close();

   exit(1);
}

////////////////////////////////////////////////////////////////////////

static void usage() {

   cout << "\nUsage: "
        << program_name << "\n"
        << "\tinput1 ... inputn\n"
        << "\tnetcdf_file\n"
        << "\t[-inputrx reg_exp]\n"
        << "\t[-format type]\n"
        << "\t[-config file]\n"
        << "\t[-mask_grid string]\n"
        << "\t[-mask_poly file]\n"
        << "\t[-mask_sid file|list]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n"
        << "\t[-valid_beg time]\n"
        << "\t[-valid_end time]\n"
        << "\t[-compress level]\n\n"

        << "\twhere\t\"input1 ... inputn\" defines one or more sources "
        << "of formatted ASCII observation files (required).\n"
        << "\t\t   Each input is the path to a file, an ASCII file list, "
        << "or a top-level directory to be recursively searched.\n"

        << "\t\t\"netcdf_file\" indicates the name of the output "
        << "NetCDF file to be written (required).\n"

        << "\t\t\"-inputrx reg_exp\" overrides the default regular "
        << "expression (" << default_reg_exp << ") when searching "
        << "directories for input files (optional).\n"

        << "\t\t\"-format type\" may be set to one of the following types (optional).\n"
        << "\t\t   "
        << MetHandler::getFormatString() << ", "
        << LittleRHandler::getFormatString() << ", "
        << SurfradHandler::getFormatString() << ", "
        << WwsisHandler::getFormatString() << ",\n\t\t   "
        << AirnowHandler::getFormatStringDailyV2() << ", "
        << AirnowHandler::getFormatStringHourlyAqObs() << ", "
        << AirnowHandler::getFormatStringHourly() << ",\n\t\t   "
        << NdbcHandler::getFormatStringStandard() << ", "
        << IsmnHandler::getFormatString() << ", "
        << IabpHandler::getFormatString() << ", "
        << UscrnHandler::getFormatString() << ",\n\t\t   "
        << AeronetHandler::getFormatString() << ", "
        << AeronetHandler::getFormatString_v2() << ", "
        << AeronetHandler::getFormatString_v3();

   #ifdef ENABLE_PYTHON
   cout << ",\n\t\t   "
        << PythonHandler::getFormatString();
   #endif

   cout << "\n"

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

        << "\t\t\"-valid_beg time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "beginning of the processed data time window (optional).\n"

        << "\t\t\"-valid_end time\" in YYYYMMDD[_HH[MMSS]] sets the "
        << "end of the processed data time window (optional).\n"

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

}

////////////////////////////////////////////////////////////////////////

static void set_inputrx(const StringArray & a) {
   input_reg_exp = a[0];
}

////////////////////////////////////////////////////////////////////////

static void set_format(const StringArray & a) {

   if(MetHandler::getFormatString() == a[0]) {
      ascii_format = ASCIIFormat::MET;
   }
   else if(LittleRHandler::getFormatString() == a[0]) {
     ascii_format = ASCIIFormat::Little_R;
   }
   else if(SurfradHandler::getFormatString() == a[0]) {
     ascii_format = ASCIIFormat::SurfRad;
   }
   else if(WwsisHandler::getFormatString() == a[0]) {
     ascii_format = ASCIIFormat::WWSIS;
   }
   else if(AirnowHandler::getFormatStringDailyV2() == a[0]) {
     ascii_format = ASCIIFormat::Airnow_dailyv2;
   }
   else if(AirnowHandler::getFormatStringHourlyAqObs() == a[0]) {
     ascii_format = ASCIIFormat::Airnow_hourlyaqobs;
   }
   else if(AirnowHandler::getFormatStringHourly() == a[0]) {
     ascii_format = ASCIIFormat::Airnow_hourly;
   }
   else if(NdbcHandler::getFormatStringStandard() == a[0]) {
     ascii_format = ASCIIFormat::NDBC_standard;
   }
   else if(IsmnHandler::getFormatString() == a[0]) {
     ascii_format = ASCIIFormat::ISMN;
   }
   else if(IabpHandler::getFormatString() == a[0]) {
     ascii_format = ASCIIFormat::IABP;
   }
   else if(UscrnHandler::getFormatString() == a[0]) {
     ascii_format = ASCIIFormat::USCRN;
   }
   else if(AeronetHandler::getFormatString() == a[0]
     || AeronetHandler::getFormatString_v2() == a[0]) {
     ascii_format = ASCIIFormat::Aeronet_v2;
   }
   else if(AeronetHandler::getFormatString_v3() == a[0]) {
     ascii_format = ASCIIFormat::Aeronet_v3;
   }
   #ifdef ENABLE_PYTHON
   else if(PythonHandler::getFormatString() == a[0]) {
     ascii_format = ASCIIFormat::Python;
   }
   #endif
   else if("python" == a[0]) {
      python_compile_error("set_format() -> ");
   }
   else {
      mlog << Error << "\nset_format() -> "
           << "unsupported ASCII observation format \""
           << a[0] << "\".\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

static void set_config(const StringArray & a) {
   config_filename = a[0];
}

////////////////////////////////////////////////////////////////////////

static void set_mask_grid(const StringArray & a) {

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

static void set_mask_poly(const StringArray & a) {
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

static void set_mask_sid(const StringArray & a) {

   // List the station ID mask
   mlog << Debug(1)
        << "Station ID Mask: " << a[0] << "\n";

   MaskSID ms = parse_sid_mask(a[0]);
   for(const auto &pair : ms.sid_map()) mask_sid.add(pair.first);

   // List the length of the station ID mask
   mlog << Debug(2)
        << "Parsed Station ID Mask (" << ms.name()
        << ") containing " << mask_sid.n() << " stations\n";
}

////////////////////////////////////////////////////////////////////////

static void set_valid_beg_time(const StringArray & a) {
   valid_beg_ut = timestring_to_unix(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

static void set_valid_end_time(const StringArray & a) {
   valid_end_ut = timestring_to_unix(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

static void set_compress(const StringArray & a) {
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

static void setup_wrapper_path() {

   #ifdef ENABLE_PYTHON
   ConcatString command;

   GP.initialize();

   run_python_string("import sys");

   command << cs_erase
           << "sys.path.append(\""
           << replace_path(pyembed_dir)
           << "\")";

   run_python_string(command.text());
   #endif

   return;
}

////////////////////////////////////////////////////////////////////////

