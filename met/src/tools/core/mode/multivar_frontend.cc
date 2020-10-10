

////////////////////////////////////////////////////////////////////////


// static const char f_multivar_program [] = "#1 && #2 && #3";
// static const char o_multivar_program [] = "#1 && #2 && #3";

static const char fcst_super_nc_filename [] = "f_super.nc";
static const char  obs_super_nc_filename [] = "o_super.nc";

static const char mode_default_config [] = "MET_BASE/config/MODEConfig_default";


////////////////////////////////////////////////////////////////////////


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
#include <fstream>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <cstdio>
#include <cmath>

#include <netcdf>

#include "vx_util.h"
#include "two_d_array.h"
#include "get_filenames.h"
#include "mode_conf_info.h"
#include "shapedata.h"
#include "interest.h"

#include "combine_boolplanes.h"
#include "objects_from_netcdf.h"


using namespace netCDF;


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

// static const char mode_path [] = "/d3/personal/randy/github/feature_1184_dryline/MET/met/src/tools/core/mode/mode";   //  hardwired for now

// static const char mode_path [] = "/d3/personal/randy/github/new/MET/met/src/tools/core/mode/mode";   //  hardwired for now

static const char sep [] = "====================================================\n";

static const char tab [] = "   ";

static const bool do_clusters = false;

static ModeConfInfo config;


////////////////////////////////////////////////////////////////////////


extern void usage();


////////////////////////////////////////////////////////////////////////


static void read_config(const string & filename);

static void get_filename_list(const char * fof_name, StringArray &);

static void run_command(const ConcatString & command);

static void replace_data(const BoolPlane & bp, const char * fcst_super_nc_filename);


////////////////////////////////////////////////////////////////////////


int multivar_frontend(const StringArray & Argv)

{

const int Argc = Argv.n();

program_name = get_short_name(Argv[0].c_str());

if ( Argc < 4 )  usage();

int j, k;
StringArray fcst_filenames;
StringArray  obs_filenames;
string config_file;
const string    mode_path = Argv[0];
const string    fcst_fof  = Argv[1];
const string     obs_fof  = Argv[2];


config_file = Argv[3];

read_config(config_file);

if ( config.fcst_multivar_logic.empty() )  {

   mlog << Error
        << program_name << ": fcst multivar logic not specified!\n\n";

   exit ( 1 );

}


if ( config.obs_multivar_logic.empty() )  {

   mlog << Error
        << program_name << ": obs multivar logic not specified!\n\n";

   exit ( 1 );

}

  //
  //  make sure the multivar logic programs are in the config file
  //

// cout << "fcst_multivar_logic = \"" << config.fcst_multivar_logic << "\"\n";
// cout << " obs_multivar_logic = \"" << config.obs_multivar_logic  << "\"\n";


get_filename_list(fcst_fof.c_str(), fcst_filenames);
get_filename_list( obs_fof.c_str(),  obs_filenames);


if ( fcst_filenames.n() != obs_filenames.n() )  {

   mlog << Error
        << "\n\n  " << program_name 
        << ": number of fcst and obs files should be the same!\n\n";

   exit ( 1 );

}

const int n_files = fcst_filenames.n();
ConcatString mode_args;
ConcatString command;
StringArray a, nc_files;
char dir [32];

mlog << Debug(1) << '\n' << sep << '\n';

   //
   //  do the individual mode runs
   //

for (j=0; j<n_files; ++j)  {

   mlog << Debug(1) << "\n starting mode run " << (j + 1) << " of " << n_files << ' ' << sep << '\n';

   command << cs_erase
           << mode_path << ' '
           << fcst_filenames[j] << ' '
           <<  obs_filenames[j] << ' '
           << config_file;

   for (k=4; k<Argc; ++k)  {

      command << ' ' << Argv[k];

   }

   snprintf(dir, sizeof(dir), "%02d", j);

   command << " -outdir " << dir;

   command << " -field_index " << j;

   // cout << "\n\n  command = \"" << command.text() << "\"\n\n" << flush;

   run_command(command);

   mlog << Debug(1) << "\n finished mode run " << (j + 1) << " of " << n_files << ' ' << sep << '\n';

   a = get_filenames_from_dir(dir, "mode_", ".nc");

   nc_files.add(a);

}   //  for j

mlog << Debug(1) << "\n finished with individual mode runs " << sep << '\n';

// nc_files.dump(cout, 1);

BoolPlane * f_plane = new BoolPlane [n_files];
BoolPlane * o_plane = new BoolPlane [n_files];
BoolPlane f_result, o_result;
// char junk[256];
Pgm image;

   //
   //  load the objects from the mode output files
   //

for (j=0; j<n_files; ++j)  {

   objects_from_netcdf(nc_files[j].c_str(), do_clusters, f_plane[j], o_plane[j]);  

/*
   snprintf(junk, sizeof(junk), "f_%02d.pgm", j);

   boolplane_to_pgm(f_plane[j], image);

   image.write(junk);
  
   snprintf(junk, sizeof(junk), "o_%02d.pgm", j);

   boolplane_to_pgm(o_plane[j], image);

   image.write(junk);
*/

}

   //
   //  combine the objects into super-objects
   //

// cout << "f = \"" << conf.

BoolCalc f_calc, o_calc ;
const int nx = f_plane[0].nx();
const int ny = f_plane[0].ny();

f_calc.set(config.fcst_multivar_logic.text());
o_calc.set(config.obs_multivar_logic.text());

// f_calc.dump_program(cout);
// o_calc.dump_program(cout);

f_result.set_size(nx, ny);
o_result.set_size(nx, ny);

combine_boolplanes(f_plane, n_files, f_calc, f_result);
combine_boolplanes(o_plane, n_files, o_calc, o_result);

boolplane_to_pgm(f_result, image);

image.write("f_result.pgm");

boolplane_to_pgm(o_result, image);

image.write("o_result.pgm");

   //
   //  copy one of the input mode files to
   //    hold the super-object data
   //

command << cs_erase << "cp -u " << fcst_filenames[0] << ' ' << fcst_super_nc_filename;

run_command(command);

command << cs_erase << "cp -u " <<  obs_filenames[0] << ' ' <<  obs_super_nc_filename;

run_command(command);

command << cs_erase << "chmod u+rw " << fcst_super_nc_filename;

run_command(command);

command << cs_erase << "chmod u+rw " << obs_super_nc_filename;

run_command(command);

   //
   //  replace the data
   //

replace_data(f_result, fcst_super_nc_filename);
replace_data(o_result,  obs_super_nc_filename);


   //
   //  done
   //

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void get_filename_list(const char * fof_name, StringArray & a)

{

ifstream in;
char line[PATH_MAX + 10];

in.open(fof_name);

if ( ! in )  {

   mlog << Error
        << "\n\n  " << program_name << ": get_filename_list() -> "
        << "unable to open input file \"" << fof_name << "\"\n\n";

   exit ( 1 );

}

while ( in.getline(line, sizeof(line)) )  {

   a.add((string) line);

}




return;

}


////////////////////////////////////////////////////////////////////////


void run_command(const ConcatString & command)

{

int status;

cout << "command = \"" << command << "\"\n" << flush;

status = system(command.text());

if ( status )  {

   mlog << Error
        << "\n\n  " << program_name << ": "
        << "command \"" << command << "\" failed!\n\n";

   exit ( 1 );

}

return;

}


////////////////////////////////////////////////////////////////////////


void replace_data(const BoolPlane & bp, const char * fcst_super_nc_filename)

{

// ConcatString path;

NcFile nc((string) fcst_super_nc_filename, NcFile::write, NcFile::classic);

   //
   //  get variable name
   //

const DictionaryEntry * e = 0;

e = config.conf.lookup("fcst", DictionaryType);

Dictionary * fcst_dict = e->dict();

e = fcst_dict->lookup((string) "field");

Dictionary * field = e->dict();

e = (*field)[0];

Dictionary * d = e->dict();

e = d->lookup("name");

e->dump(cout);

ConcatString var_name = *(e->string_value());

cout << "\n\n    var_name = \"" << var_name << "\"\n\n" << flush;

   //
   //  get variable and dimensions
   //

int x, y, n;
NcVar data_var = nc.getVar(var_name.text());
// NcDim lat_dim  = nc.getDim("lat");
// NcDim lon_dim  = nc.getDim("lon");
const int nx = bp.nx();
const int ny = bp.ny();
const int nxy = nx*ny;
double value;

float * buf = new float [nxy];

for (x=0; x<nx; ++x)  {

   for (y=0; y<ny; ++y)  {

      n = y*nx + x;

      value = ( (bp(x, y)) ? 100.0 : 0.0 );

      buf[n] = (float) value;

   }   //  for y

}   //  for x

data_var.putVar(buf);




   //
   //  done
   //

nc.close();

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


