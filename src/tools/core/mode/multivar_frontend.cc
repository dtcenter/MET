// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


static const char fcst_super_nc_filename [] = "f_super.nc";
static const char  obs_super_nc_filename [] = "o_super.nc";

static const char mode_default_config [] = "MET_BASE/config/MODEConfig_default";

static const char super_object_var_name [] = "super";

static const int dir_creation_mode = 0755;       

static const float  on_value = (float) 100.0;
static const float off_value = (float)   0.0;


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <cmath>

#include <vector>

#include <netcdf>

#include "vx_util.h"
#include "file_exists.h"
#include "two_d_array.h"
#include "get_filenames.h"
#include "mode_conf_info.h"
#include "shapedata.h"
#include "interest.h"
#include "met_file.h"
#include "mode_usage.h"
#include "mode_exec.h"

#include "combine_boolplanes.h"
#include "objects_from_netcdf.h"
#include "parse_file_list.h"


using namespace netCDF;


////////////////////////////////////////////////////////////////////////


extern int mode_frontend(const StringArray &);


////////////////////////////////////////////////////////////////////////


extern const char * const program_name;


static const char sep [] = "====================================================";

static const char tab [] = "   ";

static const bool do_clusters = false;

static ModeConfInfo config;

static string   mode_path;
static string    fcst_fof;
static string     obs_fof;
static string config_file;
static string      outdir;


////////////////////////////////////////////////////////////////////////


static void set_outdir    (const StringArray &);
static void set_logfile   (const StringArray &);
static void set_verbosity (const StringArray &);


static void read_config(const string & filename);

static void run_command(const ConcatString & command);

static void process_command_line(const StringArray &);

static void write_output_nc_file(const char * path, const MetNcFile &, const BoolPlane &);


////////////////////////////////////////////////////////////////////////


int multivar_frontend(const StringArray & Argv)

{

const int Argc = Argv.n();

if ( Argc < 4 )  multivar_usage();

int j;
StringArray fcst_filenames;
StringArray  obs_filenames;
ConcatString dir;


process_command_line(Argv);


read_config(config_file);

if ( config.fcst_multivar_logic.empty() )  {

   mlog << Error << "\n" << program_name
        << ": fcst multivar logic not specified!\n\n";

   exit ( 1 );

}


if ( config.obs_multivar_logic.empty() )  {

   mlog << Error << "\n" << program_name
        << ": obs multivar logic not specified!\n\n";

   exit ( 1 );

}

  //
  //  make sure the multivar logic programs are in the config file
  //

fcst_filenames = parse_ascii_file_list(fcst_fof.c_str());
 obs_filenames = parse_ascii_file_list(obs_fof.c_str());

if ( fcst_filenames.n() != obs_filenames.n() )  {

   mlog << Error << "\n" << program_name
        << ": number of fcst and obs files should be the same!\n\n";

   exit ( 1 );

}

const int n_files = fcst_filenames.n();
ConcatString mode_args;
ConcatString command;
StringArray a, nc_files, mode_argv;
int status;
char junk [256];

mlog << Debug(2) << "\n" << sep << "\n";

   //
   //  check for no inputs
   //

if ( n_files == 0 )  {

   mlog << Error << "\n" << program_name
        << ": no input forecast files to process!\n\n";

   exit ( 1 );

}

   //
   //  do the individual mode runs
   //

for (j=0; j<n_files; ++j)  {

   mlog << Debug(2) 
        << "\n starting mode run " << (j + 1) << " of " << n_files
        << "\n" << sep << "\n";

      //
      //  test to see of the output directory for this
      //    mode runs exists, and if not, create it
      //

   dir.clear();

   if ( outdir.length() > 0 )  dir << outdir << '/';

   snprintf(junk, sizeof(junk), "%02d", j);

   dir << junk;

   if ( ! directory_exists(dir.c_str()) )  {

      mlog << Debug(2)
           << program_name << ": creating output directory \""
           << dir << "\"\n\n";

      status = mkdir(dir.c_str(), dir_creation_mode);       

      if ( status < 0 )  {

         mlog << Error << "\n" << program_name
              << ": unable to create output directory \""
              << dir << "\"\n\n";

         exit ( 1 );

      }

   }

      //
      //  build the command for running mode
      //

   mode_argv.clear();

   mode_argv.add(mode_path);
   mode_argv.add(fcst_filenames[j]);
   mode_argv.add(obs_filenames[j]);
   mode_argv.add(config_file);

   command << cs_erase
           << mode_path         << ' '
           << fcst_filenames[j] << ' '
           <<  obs_filenames[j] << ' '
           << config_file;

   mode_argv.add("-v");
   snprintf(junk, sizeof(junk), "%d", mlog.verbosity_level());
   mode_argv.add(junk);

   mode_argv.add("-outdir");
   mode_argv.add(dir);

   command << " -v " << mlog.verbosity_level();

   command << " -outdir " << dir;

   mode_argv.add("-field_index");
   snprintf(junk, sizeof(junk), "%d", j);
   mode_argv.add(junk);

   command << " -field_index " << j;

      //
      //  run mode
      //

   mlog << Debug(1) << "Running mode command: \"" << command << "\"\n\n";

   run_command(command);

   // [TODO] MET #1238: run MODE in memory instead of via system calls.
   // (void) mode_frontend(mode_argv);

   mlog << Debug(2) << "\n finished mode run " << (j + 1) << " of " << n_files
        << "\n" << sep << "\n";

   a = get_filenames_from_dir(dir.text(), "mode_", ".nc");

   nc_files.add(a);

}   //  for j

mlog << Debug(2) << "\n finished with individual mode runs "
     << "\n" << sep << "\n";

BoolPlane * f_plane = new BoolPlane [n_files];
BoolPlane * o_plane = new BoolPlane [n_files];
BoolPlane f_result, o_result;
Pgm image;

   //
   //  load the objects from the mode output files
   //

for (j=0; j<n_files; ++j)  {

   objects_from_netcdf(nc_files[j].c_str(), do_clusters, f_plane[j], o_plane[j]);  

}

   //
   //  combine the objects into super-objects
   //

BoolCalc f_calc, o_calc ;
const int nx = f_plane[0].nx();
const int ny = f_plane[0].ny();

f_calc.set(config.fcst_multivar_logic.text());
o_calc.set(config.obs_multivar_logic.text());

f_result.set_size(nx, ny);
o_result.set_size(nx, ny);

combine_boolplanes(f_plane, n_files, f_calc, f_result);
combine_boolplanes(o_plane, n_files, o_calc, o_result);


   //
   //  write the output files
   //

MetNcFile met;   //  mostly to get grid
ConcatString path;


path = nc_files[0];

if ( ! met.open(path.text()) )  {

   mlog << Error << "\n" << program_name
        << ": unable to open mode output file \""
        << path << "\"\n\n";

   exit ( 1 );

}

ConcatString fcst_file, obs_file;
fcst_file << outdir << "/" << fcst_super_nc_filename;
 obs_file << outdir << "/" <<  obs_super_nc_filename;

write_output_nc_file(fcst_file.text(), met, f_result);
write_output_nc_file( obs_file.text(), met, o_result);

   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void set_outdir(const StringArray & a)

{

outdir = a[0];

return;

}


////////////////////////////////////////////////////////////////////////


void set_logfile(const StringArray & a)

{

ConcatString filename;

filename = a[0];

mlog.open_log_file(filename);

return;

}


////////////////////////////////////////////////////////////////////////


void set_verbosity (const StringArray & a)

{

mlog.set_verbosity_level(atoi(a[0].c_str()));

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  might want to replace this with a fork/exec
   //


void run_command(const ConcatString & command)

{

int status;

status = system(command.text());

if ( status )  {

   mlog << Error << "\n" << program_name << ": "
        << "command \"" << command << "\" failed!\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void read_config(const string & filename)

{

ConcatString path;

path = replace_path(mode_default_config);

config.read_config(path.c_str(), filename.c_str());

return;

}


////////////////////////////////////////////////////////////////////////


void process_command_line(const StringArray & argv)

{

CommandLine cline;

mode_path = argv[0];

cline.set(argv);

cline.set_usage(multivar_usage);

cline.add(set_outdir,    "-outdir", 1);
cline.add(set_logfile,   "-log",    1);
cline.add(set_verbosity, "-v",      1);

cline.parse();

   //
   //  should be 3 arguments left
   //


   fcst_fof = cline[0];
    obs_fof = cline[1];
config_file = cline[2];


return;

}


////////////////////////////////////////////////////////////////////////


void write_output_nc_file(const char * path, const MetNcFile & met, const BoolPlane & bp)

{

NcFile nc;
int x, y, n;
float * data = 0;
const Grid & grid = met.grid;

mlog << Debug(1)
     << "Creating NetCDF Output file: " << path << "\n";

nc.open(string(path), NcFile::replace, NcFile::classic);

   //  load the data

data = new float [(grid.nx())*(grid.ny())];

NcDim lat_dim;
NcDim lon_dim;
NcVar var;
const int nx = grid.nx();
const int ny = grid.ny();
vector<NcDim> vdim(2);


for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      n = y*nx + x;   //  should we just assume this ordering?

      if ( bp(x, y) )  data[n] =  on_value;
      else             data[n] = off_value;

   }   //  for x

}   //  for x

   //
   //  add global attributes and projection information
   //

write_netcdf_global(&nc, path, program_name);

   //
   //  add dimensions and write projection info
   //

write_netcdf_proj(&nc, grid, lat_dim, lon_dim);

   //
   //  variable
   //

vdim[0] = lat_dim;
vdim[1] = lon_dim;

var = nc.addVar(string(super_object_var_name), ncFloat, vdim);

var.putVar(data);

   //
   //  done
   //

nc.close();

if ( data )  { delete [] data;  data = 0; }

return;

}


////////////////////////////////////////////////////////////////////////
