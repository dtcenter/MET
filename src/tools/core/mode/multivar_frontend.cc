// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


// for multivar mode, this is the default file
static const char mode_default_config [] = "MET_BASE/config/MODEMultivarConfig_default";

static const int dir_creation_mode = 0755;       

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
#include <algorithm>

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
#include "mode_data_type.h"

using namespace netCDF;


////////////////////////////////////////////////////////////////////////


extern const char * const program_name;


static const char sep [] = "====================================================";

static const char tab [] = "   ";

// this is hardwired for the multivar case, at least for now

static const bool do_clusters = false;

static string default_out_dir = ".";

static ModeConfInfo config;

static string   mode_path;
static string    fcst_fof;
static string     obs_fof;
static string config_file;
static string      outdir;

static Grid verification_grid;

////////////////////////////////////////////////////////////////////////


static void set_outdir    (const StringArray &);
static void set_logfile   (const StringArray &);
static void set_verbosity (const StringArray &);

static void multivar_consistency_checks(StringArray &fcst_filenames, StringArray &obs_filenames,
                                        BoolCalc &f_calc, BoolCalc &o_calc, int &n_fcst_files,
                                        int &n_obs_files);

static ConcatString set_multivar_dir();

static void create_verification_grid(const string &fcst_filename,
                                     const string &obs_filename,
                                     const ConcatString &dir);

static MultiVarData *create_simple_objects(ModeDataType dtype, int j, int n_files,
                                           const string &filename, 
                                           const ConcatString &dir);

static void create_superobjects(int n_fcst_files, const vector<MultiVarData *> &mvdFcst,
                                int n_obs_files, const vector<MultiVarData *> &mvdObs,
                                BoolCalc &f_calc, BoolCalc &o_calc,
                                BoolPlane &f_simple_result, BoolPlane &o_simple_result,
                                ShapeData &f_simple_sd, ShapeData &o_simple_sd,
                                ShapeData &f_merge_sd_split, ShapeData &o_merge_sd_split);

static void create_intensity_comparisons(int findex, int oindex, const BoolPlane &f_result,
                                         BoolPlane &o_result, 
                                         const ConcatString &dir,
                                         MultiVarData &mvdf, MultiVarData &mvdo,
                                         bool has_union_f, bool has_union_o,
                                         const string &fcst_filename, const string &obs_filename,
                                         ShapeData &merge_f, ShapeData &merge_o);

static void process_superobjects(ShapeData &f_result, ShapeData &o_result,
                                 ShapeData &f_merge, ShapeData &o_merge,
                                 int nx, int ny, const ConcatString &dir,
                                 GrdFileType ftype, GrdFileType otype, const Grid &grid, bool has_union);

static void mask_data(const string &name, int nx, int ny, const BoolPlane &mask, DataPlane &data);
static void mask_data_super(const string &name, int nx, int ny, DataPlane &data);

static void read_config(const string & filename);

static void process_command_line(const StringArray &);

static int  _mkdir(const char *dir);

static void _debug_shape_examine(string &name, const ShapeData &sd, int nx, int ny);


////////////////////////////////////////////////////////////////////////


int multivar_frontend(const StringArray & Argv)

{

   const int Argc = Argv.n();

   if ( Argc < 4 )  multivar_usage();

   int j, n_fcst_files, n_obs_files;
   StringArray fcst_filenames;
   StringArray  obs_filenames;
   BoolCalc f_calc, o_calc ;

   process_command_line(Argv);

   read_config(config_file);

   multivar_consistency_checks(fcst_filenames, obs_filenames, f_calc, o_calc,
                               n_fcst_files, n_obs_files);

   bool f_has_union = f_calc.has_union();
   bool o_has_union = o_calc.has_union();

   mlog << Debug(2) << "\n" << sep << "\n";

   ConcatString dir = set_multivar_dir();

   // first thing to do is to define the verification grid using the 0th
   // fcst and obs inputs
   mlog << Debug(2) << "\n creating the verification grid \n" << sep << "\n";

   create_verification_grid(fcst_filenames[0], obs_filenames[0], dir);

   //
   // do the individual mode runs which produce everything needed to create
   // super objects (stored in 'mvdObs', 'mvdFcst') (both simple and merge steps
   // are done here).
   //

   vector<MultiVarData *> mvdObs, mvdFcst;

   for (j=0; j<n_fcst_files; ++j)  {

      mlog << Debug(2) 
           << "\n" << sep << "\ncreating simple forecast objects from forecast " << (j + 1) << " of " << n_fcst_files << "\n" << sep << "\n";
      MultiVarData *mvdi = create_simple_objects(ModeDataType_MvMode_Fcst, j, n_fcst_files, fcst_filenames[j], dir);
      if (j > 0) {
         mvdFcst[0]->checkFileTypeConsistency(*mvdi, j);
      }
      mvdFcst.push_back(mvdi);
      mvdi->print();
   }   //  for j

   for (j=0; j<n_obs_files; ++j)  {

      mlog << Debug(2) 
           << "\n" << sep << "\ncreating simple obs objects from obs " << (j + 1) << " of " << n_obs_files << "\n" << sep << "\n";
      MultiVarData *mvdi = create_simple_objects(ModeDataType_MvMode_Obs, j, n_obs_files, obs_filenames[j], dir);
      if (j > 0) {
         mvdObs[0]->checkFileTypeConsistency(*mvdi, j);
      }
      mvdObs.push_back(mvdi);
      mvdi->print();
   }   //  for j

   mlog << Debug(2) << "\n finished with simple multivar mode runs " << "\n" << sep << "\n";

   // now create forecast and obs superobjects
   
   BoolPlane f_simple_result, o_simple_result;
   ShapeData f_simple_sd, o_simple_sd, f_merge_sd_split, o_merge_sd_split;

   create_superobjects(n_fcst_files, mvdFcst, n_obs_files, mvdObs,
                       f_calc, o_calc, f_simple_result, o_simple_result,
                       f_simple_sd, o_simple_sd,
                       f_merge_sd_split, o_merge_sd_split);

   //
   // Filter the data to within the superobjects only and do statistics by invoking mode algorithm again
   // on the masked data pairs
   //
   for (int k=0; k<config.fcst_multivar_compare_index.n(); ++k)
   {
      int findex = config.fcst_multivar_compare_index[k] - 1;
      int oindex = config.obs_multivar_compare_index[k] - 1;
      create_intensity_comparisons(findex, oindex, f_simple_result, o_simple_result, dir,
                                   *mvdFcst[findex], *mvdObs[oindex], f_has_union, o_has_union,
                                   fcst_filenames[findex], obs_filenames[oindex],
                                   f_merge_sd_split, o_merge_sd_split);
   }

   mlog << Debug(2) << "\n finished with multivar intensity comparisons \n" << sep << "\n";

   if (config.fcst_multivar_compare_index.n() <= 0) {

      // grab all the info needed for superobject mode

      int nx = mvdFcst[0]->_nx;
      int ny = mvdFcst[0]->_ny;
      GrdFileType ftype = mvdFcst[0]->_type;
      GrdFileType otype = mvdObs[0]->_type;
      Grid grid = *(mvdFcst[0]->_grid);

      // here run one more time using superobjects as input

      bool has_union = f_calc.has_union() || o_calc.has_union();

      process_superobjects(f_simple_sd, o_simple_sd, f_merge_sd_split, o_merge_sd_split,
                           nx, ny, dir, ftype, otype, grid, has_union);
   }
   
   // free up memory

   for (j=0; j<n_fcst_files; ++j)  {
      delete mvdFcst[j];
      mvdFcst[j] = 0;
   }
   for (j=0; j<n_obs_files; ++j)  {
      delete mvdObs[j];
      mvdObs[j] = 0;
   }
   
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

   //
   // Set the default output directory
   //

   outdir = replace_path(default_out_dir);

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
                                 BoolCalc &f_calc, BoolCalc &o_calc, int &n_fcst_files,
                                 int &n_obs_files)
{

   //
   //  make sure the multivar logic programs are in the config file
   //

   if ( config.fcst_multivar_logic.empty() )  {

      mlog << Error << "\n" << program_name
           << ": fcst multivar logic not specified in multivar mode!\n\n";
      exit ( 1 );

   }

   if ( config.obs_multivar_logic.empty() )  {

      mlog << Error << "\n" << program_name
           << ": obs multivar logic not specified in multivar mode!\n\n";
      exit ( 1 );

   }

   fcst_filenames = parse_ascii_file_list(fcst_fof.c_str());
   obs_filenames = parse_ascii_file_list(obs_fof.c_str());

   n_fcst_files = fcst_filenames.n();
   n_obs_files = obs_filenames.n();

   //
   //  check for multivar being actually multi.
   //
   if ( n_fcst_files < 2 && n_obs_files < 2) {

      mlog << Error << "\n" << program_name
           << ": want at least 2 input files for fcst or obs in multivar mode, neither had 2 or more\n\n";
      exit ( 1 );
   }

   //
   // set values in the f_calc and o_calc objects, check that the logic is in range and the right length
   //

   f_calc.set(config.fcst_multivar_logic.text());
   o_calc.set(config.obs_multivar_logic.text());

   if (!f_calc.check_args(n_fcst_files)) {
      exit ( 1 );
   }

   if (!o_calc.check_args(n_obs_files)) {
      exit ( 1 );
   }


   if (config.fcst_multivar_compare_index.n() != config.obs_multivar_compare_index.n()) {
      mlog << Error << "\n" << program_name
           << " Need equal number of multivar_compare_index entries for obs and fcst\n\n";
      exit(1);
   }

   bool badIndex = false;
   for (int k=0; k<config.fcst_multivar_compare_index.n(); ++k)
   {
      int findex = config.fcst_multivar_compare_index[k];
      int oindex = config.obs_multivar_compare_index[k];
      if (findex <= 0 || findex > n_fcst_files) {
         mlog << Error << "\n" << program_name
              << " forecast index " << findex
              << " out of range, " << conf_key_fcst_multivar_compare_index << " array\n";
         badIndex = true;
      }
      if (oindex <= 0 || oindex > n_obs_files) {
         mlog << Error << "\n" << program_name
              << " obs index " << oindex
              << " out of range, " << conf_key_obs_multivar_compare_index << " array\n";
         badIndex = true;
      }
   }
   if (badIndex) {
      mlog << Error << "\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

ConcatString set_multivar_dir()
{
   ConcatString dir;
   int status;

   dir.clear();

   // no longer want numbered subdirectories
   if ( outdir.length() > 0 )  dir << outdir;

   //
   //  test to see of the output directory for this
   //    mode runs exists, and if not, create it
   //

   if ( ! directory_exists(dir.c_str()) )  {

      mlog << Debug(2)
           << program_name << ": creating output directory \""
           << dir << "\"\n\n";

      status = _mkdir(dir.c_str());

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

void create_verification_grid(const string &fcst_filename, const string &obs_filename,
                              const ConcatString &dir)
{
   ConcatString command;
   StringArray a, mode_argv;

   //
   //  build the command for running mode frontend
   //

   mode_argv.clear();
   mode_argv.add(mode_path);
   mode_argv.add(fcst_filename);
   mode_argv.add(obs_filename);
   mode_argv.add(config_file);

   command << cs_erase
           << mode_path     << ' '
           << fcst_filename << ' '
           << obs_filename << ' '
           << config_file;

   mode_argv.add("-v");
   char junk [256];
   snprintf(junk, sizeof(junk), "%d", mlog.verbosity_level());
   mode_argv.add(junk);

   mode_argv.add("-outdir");
   mode_argv.add(dir);

   command << " -v " << mlog.verbosity_level();
   command << " -outdir " << dir;

   //
   //  run the pass1 portions of mode, which creates simple objects
   //

   mlog << Debug(3) << "Running mode command: \"" << command << "\"\n\n";
   ModeFrontEnd *frontend = new ModeFrontEnd;
   verification_grid = frontend->create_verification_grid(mode_argv);
   delete frontend;


}

////////////////////////////////////////////////////////////////////////

MultiVarData *create_simple_objects(ModeDataType dtype,  int j, int n_files,
                                    const string &filename,
                                    const ConcatString &dir)
{
   ConcatString command;
   StringArray a, mode_argv;

   //
   //  build the command for running mode frontend
   //

   mode_argv.clear();
   mode_argv.add(mode_path);
   mode_argv.add(filename);
   mode_argv.add(config_file);

   command << cs_erase
           << mode_path     << ' '
           << filename << ' '
           << config_file;

   mode_argv.add("-v");
   char junk [256];
   snprintf(junk, sizeof(junk), "%d", mlog.verbosity_level());
   mode_argv.add(junk);

   mode_argv.add("-outdir");
   mode_argv.add(dir);

   command << " -v " << mlog.verbosity_level();
   command << " -outdir " << dir;

   //
   //  create the simple objects, forecast or obs, from this input data file
   //

   mlog << Debug(3) << "Running mode command: \"" << command << "\"\n\n";
   ModeFrontEnd *frontend = new ModeFrontEnd;
   int status = frontend->create_multivar_simple_objects(mode_argv, dtype, verification_grid, j, n_files);
   MultiVarData *mvdi = frontend->get_multivar_data(dtype);
   delete frontend;

   //
   //  create simple merge objects
   //

   frontend = new ModeFrontEnd;
   status = frontend->create_multivar_merge_objects(mode_argv, dtype, verification_grid, j, n_files);

   // add the merge results to the mvdi object
   frontend->add_multivar_merge_data(mvdi, dtype);
   delete frontend;

   return mvdi;
}

////////////////////////////////////////////////////////////////////////

void create_superobjects(int n_fcst_files, const vector<MultiVarData *> &mvdFcst,
                         int n_obs_files, const vector<MultiVarData *> &mvdObs,
                         BoolCalc &f_calc, BoolCalc &o_calc,
                         BoolPlane &f_simple_result, BoolPlane &o_simple_result,
                         ShapeData &f_simple_sd, ShapeData &o_simple_sd,
                         ShapeData &f_merge_sd_split, ShapeData &o_merge_sd_split)
{
   //
   //  set the BoolPlane values using the mvd content
   //

   BoolPlane * f_simple_plane = new BoolPlane [n_fcst_files];
   BoolPlane * o_simple_plane = new BoolPlane [n_obs_files];
   BoolPlane * f_merge_plane = new BoolPlane [n_fcst_files];
   BoolPlane * o_merge_plane = new BoolPlane [n_obs_files];

   for (int j=0; j<n_fcst_files; ++j)  {
      mvdFcst[j]->objects_from_arrays(do_clusters, true, f_simple_plane[j]);
      mvdFcst[j]->objects_from_arrays(do_clusters, false, f_merge_plane[j]);
   }

   for (int j=0; j<n_obs_files; ++j)  {
      mvdObs[j]->objects_from_arrays(do_clusters, true, o_simple_plane[j]);
      mvdObs[j]->objects_from_arrays(do_clusters, false, o_merge_plane[j]);
   }

   //
   //  combine the objects into super-objects
   //
   const int nx = f_simple_plane[0].nx();
   const int ny = f_simple_plane[0].ny();

   BoolPlane f_merge_result, o_merge_result;
   f_simple_result.set_size(nx, ny);
   o_simple_result.set_size(nx, ny);
   f_merge_result.set_size(nx, ny);
   o_merge_result.set_size(nx, ny);

   combine_boolplanes("Fcst_Simple", f_simple_plane, n_fcst_files, f_calc, f_simple_result);
   combine_boolplanes("Obs_Simple", o_simple_plane, n_obs_files, o_calc, o_simple_result);
   combine_boolplanes("Fcst_Merge", f_merge_plane, n_fcst_files, f_calc, f_merge_result);
   combine_boolplanes("Obs_Merge", o_merge_plane, n_obs_files, o_calc, o_merge_result);


   // create ShapeData objects using something from mvd as a template
   // (shape data has 1's or bad)

   f_simple_sd = ShapeData(*(mvdFcst[0]->_simple->_sd));
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         if (f_simple_result.get(x, y)) {
            f_simple_sd.data.put(1.0, x, y);
         } else {
            f_simple_sd.data.put(bad_data_double, x, y);
         }
      }
   }
   o_simple_sd = ShapeData(*(mvdObs[0]->_simple->_sd));
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         if (o_simple_result.get(x, y)) {
            o_simple_sd.data.put(1.0, x, y);
         } else {
            o_simple_sd.data.put(bad_data_double, x, y);
         }
      }
   }

   ShapeData f_merge_sd = ShapeData(*(mvdFcst[0]->_simple->_sd));
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         if (f_merge_result.get(x, y)) {
            f_merge_sd.data.put(1.0, x, y);
         } else {
            f_merge_sd.data.put(bad_data_double, x, y);
         }
      }
   }
   
   ShapeData o_merge_sd = ShapeData(*(mvdObs[0]->_simple->_sd));
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         if (o_merge_result.get(x, y)) {
            o_merge_sd.data.put(1.0, x, y);
         } else {
            o_merge_sd.data.put(bad_data_double, x, y);
         }
      }
   }

   int n_f_shapes;
   f_merge_sd_split = split(f_merge_sd, n_f_shapes);
   string s = "Forecast Merge";
   _debug_shape_examine(s, f_merge_sd_split, nx, ny);

   int n_o_shapes;
   o_merge_sd_split = split(o_merge_sd, n_o_shapes);
   s = "Obs Merge";
   _debug_shape_examine(s, o_merge_sd_split, nx, ny);
}


////////////////////////////////////////////////////////////////////////

void
create_intensity_comparisons(int findex, int oindex, const BoolPlane &f_result,
                             BoolPlane &o_result, const ConcatString &dir,
                             MultiVarData &mvdf, MultiVarData &mvdo,
                             bool has_union_f, bool has_union_o,
                             const string &fcst_filename, const string &obs_filename,
                             ShapeData &merge_f, ShapeData &merge_o)
{

   // mask the input data to be valid only inside the simple super objects
   int nx = mvdf._nx;
   int ny = mvdf._ny;
   
   mask_data("Fcst", nx, ny, f_result, mvdf._simple->_sd->data);
   mask_data("Obs", nx, ny, o_result, mvdo._simple->_sd->data);

   //
   //  build the command for running mode frontend
   //
   StringArray mode_argv;
   char junk [256];

   mode_argv.clear();
   mode_argv.add(mode_path);
   mode_argv.add(fcst_filename);
   mode_argv.add(obs_filename);
   mode_argv.add(config_file);
   mode_argv.add("-v");
   snprintf(junk, sizeof(junk), "%d", mlog.verbosity_level());
   mode_argv.add(junk);
   mode_argv.add("-outdir");
   mode_argv.add(dir);

   mlog << Debug(1) << "Running mvmode intensity comparisions \n\n";

   ModeFrontEnd *frontend = new ModeFrontEnd;
   int status = frontend->multivar_intensity_comparisons(mode_argv, mvdf, mvdo, has_union_f,
                                                         has_union_o, merge_f, merge_o, findex,
                                                         oindex);
   delete frontend;
}

////////////////////////////////////////////////////////////////////////

void process_superobjects(ShapeData &f_result, ShapeData &o_result,
                          ShapeData &f_merge, ShapeData &o_merge,
                          int nx, int ny, const ConcatString &dir,
                          GrdFileType ftype, GrdFileType otype, const Grid &grid,
                          bool has_union)
{
   StringArray mode_argv;
   char junk [256];

   //
   //  build the command for running mode frontend
   // 
   mode_argv.clear();
   mode_argv.add(mode_path);
   mode_argv.add(config_file);
   mode_argv.add("-v");
   snprintf(junk, sizeof(junk), "%d", mlog.verbosity_level());
   mode_argv.add(junk);
   mode_argv.add("-outdir");
   mode_argv.add(dir);

   mlog << Debug(1) << "Running superobject mode \n\n";

   // set the data to 0 inside superobjects and missing everywhere else

   mask_data_super("FcstSimple", nx, ny, f_result.data);
   mask_data_super("ObsSimple", nx, ny, o_result.data);
   

   ModeFrontEnd *frontend = new ModeFrontEnd;
   int status = frontend->run_super(mode_argv, f_result, o_result,
                                    f_merge, o_merge, ftype, otype, grid, has_union);
   delete frontend;
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
   
   mlog << Debug(1) << name << " superobject masking.."
        << nkeep << " points of "
        << nmasked + nkeep << " in superobjects\n";
}



////////////////////////////////////////////////////////////////////////
void mask_data_super(const string &name, int nx, int ny, DataPlane &data)
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

         if(is_bad_data(data.get(x,y))) {
            nmasked ++;
         } else {
            data.set(0.0, x, y);
            nkeep ++;
         }
      }
   }
   
   mlog << Debug(1) << name << " superobject masking.."
        << nkeep << " points of "
        << nmasked + nkeep << " in superobjects\n";
}

////////////////////////////////////////////////////////////////////////

int _mkdir(const char *dir)
{
   char tmp[256];
   char *p = NULL;
   size_t len;

   snprintf(tmp, sizeof(tmp),"%s",dir);
   len = strlen(tmp);
   if (tmp[len - 1] == '/')
      tmp[len - 1] = 0;
   for (p = tmp + 1; *p; p++)
      if (*p == '/') {
         *p = 0;
         string s = tmp;
         if (s != ".") {
            if (mkdir(tmp, dir_creation_mode) < 0) {
               mlog << Error << "\n making " << tmp << "\n";
               return -1;
            }
         }
         *p = '/';
      }

   return (mkdir(tmp, dir_creation_mode));
}

////////////////////////////////////////////////////////////////////////

void  _debug_shape_examine(string &name, const ShapeData &sd, int nx, int ny)
{
   vector<double> values;
   vector<int> count;
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         double v = sd.data.get(x,y);
         if (v <= 0) {
            continue;
         }
         vector<double>::iterator vi;
         vi = find(values.begin(), values.end(), v);
         if (vi == values.end()) {
            values.push_back(v);
            count.push_back(1);
         } else {
            int ii = vi - values.begin();
            count[ii] = count[ii] + 1;
         }
      }
   }
   for (size_t i=0; i<values.size(); ++i) {
      mlog << Debug(1) << name << " shape value=" << values[i] << " count=" << count[i] << "\n";
   }
}   
