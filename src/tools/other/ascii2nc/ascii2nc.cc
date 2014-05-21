// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2013
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

#include "netcdf.hh"

#include "vx_nc_util.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_log.h"

#include "ascii2nc_conf_info.h"
#include "file_handler.h"
#include "little_r_handler.h"
#include "met_handler.h"
#include "surfrad_handler.h"

////////////////////////////////////////////////////////////////////////

// Constants
static const char *program_name = "ascii2nc";

static const string DEFAULT_CONFIG_FILENAME =
  "MET_BASE/data/config/Ascii2NcConfig_default";

////////////////////////////////////////////////////////////////////////

// Supported input ASCII formats
enum ASCIIFormat {
   ASCIIFormat_None,
   ASCIIFormat_MET,
   ASCIIFormat_Little_R,
   ASCIIFormat_SurfRad
};
static ASCIIFormat ascii_format = ASCIIFormat_None;

////////////////////////////////////////////////////////////////////////

// Variables for command line arguments
static vector< ConcatString > asfile_list;
static ConcatString ncfile;

static bool config_file_used = false;
static Ascii2NcConfInfo config_info;

////////////////////////////////////////////////////////////////////////

static FileHandler *create_file_handler(const ASCIIFormat,
                                        const ConcatString &);
static FileHandler *determine_ascii_format(const ConcatString &);

static void usage();
static void set_format(const StringArray &);
static void set_logfile(const StringArray &);
static void set_config(const StringArray &);
static void set_verbosity(const StringArray &);

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
   cline.add(set_format, "-format", 1);
   cline.add(set_logfile, "-log", 1);
   cline.add(set_verbosity, "-v", 1);
   cline.add(set_config, "-config", 1);
   
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
     asfile_list.push_back(cline[i]);
   ncfile = cline[cline.n() - 1];

   //
   // Create the file handler based on the ascii format specified on
   // the command line.  If one wasn't specified, we'll look in the
   // first file to guess the format.
   //
   FileHandler *file_handler = create_file_handler(ascii_format,
                                                   asfile_list[0]);
   
   if (file_handler == 0)
     return 0;
   
   //
   // Process the files.  If a configuration file was specified, do any
   // extra processing specified.
   //
   file_handler->readAsciiFiles(asfile_list);

   if (config_file_used)
   {
     // Summarize the observations, if directed.  We need to use a different
     // call to writeNetcdfFile in this case so that we can include the
     // summarization details.

     if (config_info.getSummaryInfo().flag)
     {
       file_handler->summarizeObs(config_info.getSummaryInfo());
     }
   }

   file_handler->writeNetcdfFile(ncfile.text());
   
   delete file_handler;
   
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
  switch (format)
  {
  case ASCIIFormat_MET:
  {
    return (FileHandler *)new MetHandler(program_name);
  }
  
  case ASCIIFormat_Little_R:
  {
    return (FileHandler *)new LittleRHandler(program_name);
  }
  
  case ASCIIFormat_SurfRad:
  {
    return (FileHandler *)new SurfradHandler(program_name);
  }
  
  case ASCIIFormat_None:
  {
    return determine_ascii_format(ascii_filename);
  }
  
  }

  return 0;
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

  if(!f_in.open(ascii_filename)) {
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
  
  if (met_file->isFileType(f_in))
  {
    f_in.close();
    return (FileHandler *)met_file;
  }
  
  delete met_file;
  
  //
  // See if this is a Little R file.
  //
  f_in.rewind();
  LittleRHandler *little_r_file = new LittleRHandler(program_name);
  
  if (little_r_file->isFileType(f_in))
  {
    f_in.close();
    return (FileHandler *)little_r_file;
  }
  
  delete little_r_file;
  
  //
  // See if this is a SURFRAD file.
  //
  f_in.rewind();
  SurfradHandler *surfrad_file = new SurfradHandler(program_name);
  
  if (surfrad_file->isFileType(f_in))
  {
    f_in.close();
    return (FileHandler *)surfrad_file;
  }
  
  delete surfrad_file;
  
  //
  // If we get here, we didn't recognize the file contents.
  //
  
  mlog << Error << "\ndetermine_ascii_format() -> "
       << "could not determine file format based on file contents\n\n";
  
  f_in.close();
  
  return 0;
}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\nUsage: "
        << program_name << "\n"
        << "\tascii_file1 [ascii_file2 ascii_file3 ... ascii_filen]\n"
        << "\tnetcdf_file\n"
        << "\t[-format ASCII_format]\n"
        << "\t[-config file]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"ascii_file\" is the formatted ASCII "
        << "observation file to be converted to NetCDF format "
        << "(required).\n"

        << "\t\t\"netcdf_file\" indicates the name of the output "
        << "NetCDF file to be written (required).\n"

        << "\t\t\"-format ASCII_format\" may be set to \""
        << MetHandler::getFormatString() << "\", \""
        << LittleRHandler::getFormatString() << "\" or \""
        << SurfradHandler::getFormatString() << "\" (optional).\n"

        << "\t\t\"-config file\" uses the specified configuration file "
	<< "to generate summaries of the fields in the ASCII files (optional).\n"

        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"

        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"

        << "\tThe \"" << MetHandler::getFormatString()
        << "\" ASCII format consists of 11 columns:\n"
        << "\t\tMessage_Type Station_ID Valid_Time(YYYYMMDD_HHMMSS)\n"
        << "\t\tLat(Deg North) Lon(Deg East) Elevation(msl)\n"
        << "\t\tGrib_Code Level Height(msl or agl) QC_String Observation_Value\n\n"

        << "\t\twhere\t\"Level\" is the pressure level (hPa) or "
        << "accumulation interval (HH[MMSS]).\n"
        << "\t\t\t\"Height\" is meters above sea level or above ground level for the "
        << "observation (msl or agl).\n\n"

        << "\t\t\tUse a value of \"" << bad_data_int
        << "\" to indicate missing data.\n\n"

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

void set_config(const StringArray & a)
{
  config_file_used = true;
  
  ConcatString config_filename = a[0];

  // List the config file
  mlog << Debug(1)
       << "Config File: " << config_filename << "\n";

  // Read the config files

  config_info.read_config(DEFAULT_CONFIG_FILENAME, config_filename.text());
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////
