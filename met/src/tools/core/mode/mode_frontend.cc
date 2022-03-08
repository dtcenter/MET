// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////
//
//   Filename:   mode.cc
//
//   Description:
//      Based on user specified parameters, this tool derives objects
//      within two gridded datasets using a convolution-thresholding
//      approach.  It then compares objects within the same field and
//      across fields, and calculates a total interest value for each
//      pair of objects using a fuzzy logic approach.  The interest
//      values are thresholded, and object pairs with a high enough
//      interest value are associated with one another.
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    10-11-06  Halley Gotway  New
//   001    12-20-06  Halley Gotway  Write check_xy_ll to reduce
//                                   duplicated code.
//   002    01/11/08  Halley Gotway  Modify sprintf statements which
//                                   use the GRIB code abbreviation string
//   003    02/06/08  Halley Gotway  Modify to read the updated NetCDF
//                                   output of PCP-Combine
//   004    09/23/08  Halley Gotway  Add two output fields to the
//                                   NetCDF object file for the raw fcst/obs values.
//   005    09/23/08  Halley Gotway  Change argument sequence for the
//                                   get_grib_record routine.
//   006    05/03/10  Halley Gotway  Remove the variable/level info
//                                   from the output file naming convention.
//   007    05/11/10  Halley Gotway  Plot polyline lines thicker.
//   008    06/30/10  Halley Gotway  Enhance grid equality checks.
//   009    07/27/10  Halley Gotway  Add lat/lon variables to NetCDF.
//   010    08/09/10  Halley Gotway  Add valid time variable attributes
//                                   to NetCDF output.
//   011    10/28/11  Holmes         Added use of command line class to
//                                   parse the command line arguments.
//   012    11/15/11  Holmes         Added code to enable reading of
//                                   multiple config files.
//
//   013    01/11/11  Bullock        Ported to new repository
//   014    05/10/12  Halley Gotway  Switch to using vx_config library.
//   015    05/10/12  Halley Gotway  Move -fcst_valid, -fcst_lead,
//                                   -obs_valid, and -obs_lead command line options
//                                   to config file.
//   016    02/25/15  Halley Gotway  Add automated regridding.
//   017    05/20/16  Prestopnik J   Removed -version (now in command_line.cc)
//   018    04/08/19  Halley Gotway  Add percentile thresholds.
//   019    04/01/19  Fillmore       Add FCST and OBS units.
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
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


#include "string_array.h"

#include "mode_usage.h"
#include "mode_exec.h"

#ifdef WITH_PYTHON
#include "global_python.h"
#endif


///////////////////////////////////////////////////////////////////////
//
// Create output files using the following naming convention:
//
//    mode_YYYYMMDDI_FHH_HHA.ps/.txt
//
//    Where I indicates initilization time, L indicates lead time,
//    and A indicates accumulation time.
//
///////////////////////////////////////////////////////////////////////


extern const char * const program_name;

static ModeExecutive * mode_exec = 0;


///////////////////////////////////////////////////////////////////////


static const char * default_out_dir = ".";

static int compress_level = -1;

static int field_index = -1;


///////////////////////////////////////////////////////////////////////


static void do_quilt    ();
static void do_straight ();

static void process_command_line(const StringArray &);


static void set_config_merge_file (const StringArray &);
static void set_outdir            (const StringArray &);
static void set_logfile           (const StringArray &);
static void set_verbosity         (const StringArray &);
static void set_compress          (const StringArray &);

static void set_field_index       (const StringArray &);   //  undocumented


///////////////////////////////////////////////////////////////////////


int mode_frontend(const StringArray & Argv)

{

   //
   // Set handler to be called for memory allocation error
   //

set_new_handler(oom);

   //
   // Process the command line arguments
   //

mode_exec = new ModeExecutive;

process_command_line(Argv);



   //
   // Process the forecast and observation files
   //

ModeConfInfo & conf = mode_exec->engine.conf_info;

if ( field_index >= 0 )  conf.set_field_index(field_index);

mode_exec->setup_fcst_obs_data();

if (compress_level >= 0) conf.nc_info.set_compress_level(compress_level);


if ( conf.quilt )  {

   do_quilt();

} else {

   do_straight();

}

// mode_exec.clear();

   //
   //  done
   //

#ifdef  WITH_PYTHON
   GP.finalize();
#endif

if ( mode_exec )  { delete mode_exec;  mode_exec = 0; }

return ( 0 );

}


///////////////////////////////////////////////////////////////////////


void do_straight()

{

const ModeConfInfo & conf = mode_exec->engine.conf_info;

const int NCT = conf.n_conv_threshs();
const int NCR = conf.n_conv_radii();

if ( NCT != NCR )  {

   mlog << Error
        << "\n\n  "
        << program_name
        << ": all convolution radius and threshold arrays must have the same number of elements!\n\n";

   exit ( 1 );

}

int index;

for (index=0; index<NCT; ++index)  {

   mode_exec->do_conv_thresh(index, index);

   mode_exec->do_match_merge();

   mode_exec->process_output();

}


   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////


void do_quilt()

{

int t_index, r_index;   //  indices into the convolution threshold and radius arrays


for (r_index=0; r_index<(mode_exec->n_conv_radii()); ++r_index)  {

   for (t_index=0; t_index<(mode_exec->n_conv_threshs()); ++t_index)  {

      mode_exec->do_conv_thresh(r_index, t_index);

      mode_exec->do_match_merge();

      mode_exec->process_output();

   }

}

   //
   //  done
   //

return;

}


///////////////////////////////////////////////////////////////////////


void process_command_line(const StringArray & argv)

{

   CommandLine cline;
   ConcatString s;
   const int argc = argv.n();


   // Set the default output directory
   mode_exec->out_dir = replace_path(default_out_dir);

   // Check for zero arguments
   if(argc == 1) singlevar_usage();

   // Parse the command line into tokens
   cline.set(argv);

   // Set the usage function
   cline.set_usage(singlevar_usage);

   // Add the options function calls
   cline.add(set_config_merge_file, "-config_merge", 1);
   cline.add(set_outdir,            "-outdir",       1);
   cline.add(set_logfile,           "-log",          1);
   cline.add(set_verbosity,         "-v",            1);
   cline.add(set_compress,          "-compress",     1);

      //
      //  add for mode multivar ... undocumented
      //

   cline.add(set_field_index, "-field_index", 1);

   // Parse the command line
   cline.parse();

   // Check for error. There should be three arguments left:
   // forecast, observation, and config filenames
   if(cline.n() != 3) singlevar_usage();

   // Store the input forecast and observation file names
   mode_exec->fcst_file         = cline[0];
   mode_exec->obs_file          = cline[1];
   mode_exec->match_config_file = cline[2];

   mode_exec->init();

   return;

}

///////////////////////////////////////////////////////////////////////

void set_config_merge_file(const StringArray & a)
{
   mode_exec->merge_config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray & a)
{
   mode_exec->out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a)
{
  mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void set_compress(const StringArray & a) {
  compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////


void set_field_index(const StringArray & a)

{

field_index = atoi(a[0].c_str());

if ( field_index < 0 )  {

   mlog << Error
        << program_name << ": bad index value ... "
        << field_index << "\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////



