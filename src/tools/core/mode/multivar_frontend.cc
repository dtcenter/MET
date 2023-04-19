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
#include "mode_frontend.h"
#include "multivar_data.h"

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

static void multivar_consistency_checks(StringArray &fcst_filenames, StringArray &obs_filenames,
                                        BoolCalc &f_calc, BoolCalc &o_calc, int &n_files);

static ConcatString set_multivar_dir(int j);

static MultiVarData *process_multivar_data(int j, const string &fcst_filename,
                                           const string &obs_filename,
                                           const ConcatString &dir);

static void process_masked_multivar_data(int j, const BoolPlane &f_result, const BoolPlane &o_result,
                                         int nx, int ny, const ConcatString &dir,
                                         MultiVarData &mvd, bool has_union);

static void mask_data(const string &name, int nx, int ny, const BoolPlane &mask, DataPlane &data);

static void read_config(const string & filename);

static void process_command_line(const StringArray &);

static void write_output_nc_file(const char * path, const MetNcFile &, const BoolPlane &);


////////////////////////////////////////////////////////////////////////


int multivar_frontend(const StringArray & Argv)

{

   const int Argc = Argv.n();

   if ( Argc < 4 )  multivar_usage();

   int j, n_files;
   StringArray fcst_filenames;
   StringArray  obs_filenames;
   BoolCalc f_calc, o_calc ;

   process_command_line(Argv);

   read_config(config_file);

   multivar_consistency_checks(fcst_filenames, obs_filenames, f_calc, o_calc, n_files);

   bool has_union = f_calc.has_union() || o_calc.has_union();

   mlog << Debug(2) << "\n" << sep << "\n";

   //
   //  do the individual mode runs
   //

   vector<MultiVarData *> mvd;
   StringArray nc_files;

   for (j=0; j<n_files; ++j)  {

      mlog << Debug(2) 
           << "\n starting mode run " << (j + 1) << " of " << n_files
           << "\n" << sep << "\n";
      ConcatString dir = set_multivar_dir(j);
      MultiVarData *mvdi = process_multivar_data(j, fcst_filenames[j], obs_filenames[j], dir);
      mvd.push_back(mvdi);
      mlog << Debug(2) << "\n finished mode run " << (j + 1) << " of " << n_files
           << "\n" << sep << "\n";
      StringArray a = get_filenames_from_dir(dir.text(), "mode_", ".nc");
      nc_files.add(a);

   }   //  for j

   mlog << Debug(2) << "\n finished with individual mode runs " << "\n" << sep << "\n";
   //nc_files.dump(cout, 0);

   //
   //  set the BoolPlane values using the mvd content
   //

   BoolPlane * f_plane = new BoolPlane [n_files];
   BoolPlane * o_plane = new BoolPlane [n_files];

   for (j=0; j<n_files; ++j)  {

      //objects_from_netcdf(nc_files[j].c_str(), do_clusters, f_plane[j], o_plane[j]);  
      objects_from_arrays(do_clusters, mvd[j]->fcst_obj_data, mvd[j]->obs_obj_data, mvd[j]->nx, mvd[j]->ny, f_plane[j], o_plane[j]);  

   }

   //
   //  combine the objects into super-objects
   //

   const int nx = f_plane[0].nx();
   const int ny = f_plane[0].ny();
   BoolPlane f_result, o_result;

   f_result.set_size(nx, ny);
   o_result.set_size(nx, ny);

   combine_boolplanes(f_plane, n_files, f_calc, f_result);
   combine_boolplanes(o_plane, n_files, o_calc, o_result);

   //
   // Filter the data to within the superobjects only and do statistics by invoking mode algorithm again
   // on the masked data
   //
 
   for (j=0; j<n_files; ++j) {

      if (!config.multivar_intensity[j]) {
         mlog << Debug(2) << "\n SKIPPING field " << j << " for masked intensities \n";
         continue;
      }

      mlog << Debug(2) 
           << "\n starting masked data mode run " << (j + 1) << " of " << n_files
           << "\n" << sep << "\n";

      // same here as above, overwriting contents written to previously 
      ConcatString dir = set_multivar_dir(j);

      process_masked_multivar_data(j, f_result, o_result, nx, ny, dir, *mvd[j], has_union);
      delete mvd[j];
      mvd[j] = 0;
   }
 
   mlog << Debug(2) << "\n finished with individual masked mode runs " << "\n" << sep << "\n";

   //
   //  write the superobject output files
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

void multivar_consistency_checks(StringArray &fcst_filenames, StringArray &obs_filenames,
                                 BoolCalc &f_calc, BoolCalc &o_calc, int &n_files)
{

   //
   //  make sure the multivar logic programs are in the config file
   //

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
   // make sure there are the same number of obs and fcst files
   //

   fcst_filenames = parse_ascii_file_list(fcst_fof.c_str());
   obs_filenames = parse_ascii_file_list(obs_fof.c_str());

   if ( fcst_filenames.n() != obs_filenames.n() )  {

      mlog << Error << "\n" << program_name
           << ": number of fcst and obs files should be the same!\n\n";
      exit ( 1 );

   }

   //
   //  check for multivar being actually multi.
   //

   n_files = fcst_filenames.n();
   if ( n_files < 2 )  {

      mlog << Error << "\n" << program_name
           << ": want at least 2 input forecast files in multivar mode, got " << n_files << "\n\n";
      exit ( 1 );

   }
   //
   // set values in the f_calc and o_calc objects, check that the logic is in range and the right length
   //

   f_calc.set(config.fcst_multivar_logic.text());
   o_calc.set(config.obs_multivar_logic.text());

   if (!f_calc.check_args(n_files)) {
      exit ( 1 );
   }

   if (!o_calc.check_args(n_files)) {
      exit ( 1 );
   }

   //
   // check that the multivar_intensity vector is the right length.
   // If empty set to the default of all FALSE
   //

   int num_multivar = (int)config.multivar_intensity.n();
   if (num_multivar == n_files) {
      return;
   }

   if (num_multivar == 0) {

      mlog << Warning << "\nmultivar_frontend() -> "
           << "empty multivar intensity array, setting to all FALSE \n\n";

      for (int i=0; i<n_files; ++i) {
         config.multivar_intensity.add(false);
      }

   } else {

      mlog << Error << "\n" << program_name 
           << ": wrong size multivar_intensity array, wanted "
           << n_files << " got " << num_multivar << "\n\n";
      exit ( 1 );
   }
}

////////////////////////////////////////////////////////////////////////

ConcatString set_multivar_dir(int j)
{
   ConcatString dir;
   int status;
   char junk [256];

   //
   //  test to see of the output directory for this
   //    mode runs exists, and if not, create it
   //

   dir.clear();
#ifdef OLDWAY
   if ( outdir.length() > 0 )  dir << outdir << '/';
   snprintf(junk, sizeof(junk), "%02d", j);
   dir << junk;
#else
   if ( outdir.length() > 0 )  dir << outdir;
#endif

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
   return dir;
}

////////////////////////////////////////////////////////////////////////

MultiVarData *process_multivar_data(int j, const string &fcst_filename, const string &obs_filename,
                                    const ConcatString &dir)
{
   ConcatString command;
   StringArray a, mode_argv;

   //
   //  build the command for running mode
   //

   mode_argv.clear();
   mode_argv.add(mode_path);
   mode_argv.add(fcst_filename);
   mode_argv.add(obs_filename);
   mode_argv.add(config_file);

   command << cs_erase
           << mode_path     << ' '
           << fcst_filename << ' '
           << obs_filename  << ' '
           << config_file;

   mode_argv.add("-v");
   char junk [256];
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
   //  run mode (sort of)
   //

   mlog << Debug(1) << "Running mode command: \"" << command << "\"\n\n";
   ModeFrontEnd *frontend = new ModeFrontEnd;

   int status = frontend->run(mode_argv, ModeFrontEnd::MULTIVAR_PASS1);
   MultiVarData *mvdi = frontend->get_multivar_data();
   delete frontend;
   return mvdi;
}

////////////////////////////////////////////////////////////////////////

void process_masked_multivar_data(int j, const BoolPlane &f_result,
                                  const BoolPlane &o_result, int nx, int ny,
                                  const ConcatString &dir, MultiVarData &mvd,
                                  bool has_union)
{
  
   mask_data("Fcst", nx, ny, f_result, mvd.Fcst_sd->data);
   mask_data("Obs", nx, ny, o_result, mvd.Obs_sd->data);


   StringArray mode_argv;
   char junk [256];

   //
   //  build the command for running mode again, sort of mode
   //
   mode_argv.clear();
   mode_argv.add(mode_path);
   mode_argv.add(config_file);
   mode_argv.add("-v");
   snprintf(junk, sizeof(junk), "%d", mlog.verbosity_level());
   mode_argv.add(junk);
   mode_argv.add("-outdir");
   mode_argv.add(dir);
   mode_argv.add("-field_index");
   snprintf(junk, sizeof(junk), "%d", j);
   mode_argv.add(junk);

   mlog << Debug(1) << "Running filtered mode \n\n";

   ModeFrontEnd *frontend = new ModeFrontEnd;
   int status = frontend->run(mode_argv, mvd, has_union);
   delete frontend;
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
void mask_data(const string &name, int nx, int ny, const BoolPlane &bp, DataPlane &data)
{

   if (nx != data.nx() || ny != data.ny()) {
      mlog << Error << "\n" << program_name  << ":" << name
           << " :dimensions don't match " << nx << " " <<  ny 
           << "    " << data.nx() << " " << data.ny() << "\n\n";

      exit( 1 );
   }

   int nmasked=0, nkeep=0;
   
   for (int x=0; x<nx; ++x)  {

      for (int y=0; y<ny; ++y)  {

         if ( bp(x, y) == false) {
            data.set(bad_data_float, x, y);;
            nmasked ++;
         } else {
            nkeep ++;
         }
      }
   }
   
   mlog << Debug(1) << name << " superobject masking " << nkeep << " of " << nkeep + nmasked << " data values remain unmasked\n";
}

