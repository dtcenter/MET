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


////////////////////////////////////////////////////////////////////////


static void set_outdir    (const StringArray &);
static void set_logfile   (const StringArray &);
static void set_verbosity (const StringArray &);

static void multivar_consistency_checks(StringArray &fcst_filenames, StringArray &obs_filenames,
                                        BoolCalc &f_calc, BoolCalc &o_calc, int &n_files);

static ConcatString set_multivar_dir();

static MultiVarData *create_simple_objects(int j, int n_files,
                                           const string &fcst_filename, const string &obs_filename,
                                           const ConcatString &dir);

static void create_singlevar_intensity_objects(int j, const BoolPlane &f_result,
                                               const BoolPlane &o_result,
                                               int nx, int ny, const ConcatString &dir,
                                               MultiVarData &mvd, bool has_union,
                                               ShapeData &f_merge, ShapeData &o_merge);

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

   int j, n_files;
   StringArray fcst_filenames;
   StringArray  obs_filenames;
   BoolCalc f_calc, o_calc ;

   process_command_line(Argv);

   read_config(config_file);

   multivar_consistency_checks(fcst_filenames, obs_filenames, f_calc, o_calc, n_files);

   bool has_union = f_calc.has_union() || o_calc.has_union();

   mlog << Debug(2) << "\n" << sep << "\n";

   ConcatString dir = set_multivar_dir();

   //
   // do the individual mode runs which produce everything needed to create
   // super objects (stored in 'mvd') (both simple and 'merge').
   //

   vector<MultiVarData *> mvd;

   for (j=0; j<n_files; ++j)  {

      mlog << Debug(2) 
           << "\n starting mode run " << (j + 1) << " of " << n_files << "\n" << sep << "\n";
      MultiVarData *mvdi = create_simple_objects(j, n_files, fcst_filenames[j], obs_filenames[j], dir);
      if (j > 0) {
         mvd[0]->checkFileTypeConsistency(*mvdi, j);
      }
      mvd.push_back(mvdi);
      mvdi->print();

   }   //  for j

   mlog << Debug(2) << "\n finished with individual mode runs " << "\n" << sep << "\n";

   //
   //  set the BoolPlane values using the mvd content
   //

   BoolPlane * f_simple_plane = new BoolPlane [n_files];
   BoolPlane * o_simple_plane = new BoolPlane [n_files];
   BoolPlane * f_merge_plane = new BoolPlane [n_files];
   BoolPlane * o_merge_plane = new BoolPlane [n_files];

   bool simple = true;
   for (j=0; j<n_files; ++j)  {
      mvd[j]->objects_from_arrays(do_clusters, simple, f_simple_plane[j], o_simple_plane[j]);
   }

   simple = false;
   for (j=0; j<n_files; ++j)  {
      mvd[j]->objects_from_arrays(do_clusters, simple, f_merge_plane[j], o_merge_plane[j]);
   }

   //
   //  combine the objects into super-objects
   //

   const int nx = f_simple_plane[0].nx();
   const int ny = f_simple_plane[0].ny();
   BoolPlane f_simple_result, o_simple_result, f_merge_result, o_merge_result;

   f_simple_result.set_size(nx, ny);
   o_simple_result.set_size(nx, ny);
   f_merge_result.set_size(nx, ny);
   o_merge_result.set_size(nx, ny);

   combine_boolplanes("Fcst_Simple", f_simple_plane, n_files, f_calc, f_simple_result);
   combine_boolplanes("Obs_Simple", o_simple_plane, n_files, o_calc, o_simple_result);

   combine_boolplanes("Fcst_Merge", f_merge_plane, n_files, f_calc, f_merge_result);
   combine_boolplanes("Obs_Merge", o_merge_plane, n_files, o_calc, o_merge_result);


   // might need this for a super object pass, so grab the values
   GrdFileType ftype = mvd[0]->_ftype;
   GrdFileType otype = mvd[0]->_otype;

   // might need this for a super passs, so store locally
   Grid grid = *(mvd[0]->_grid);


   // create ShapeData objects using something from mvd as a template
   // shape data with 1's or bad

   ShapeData f_simple_sd = ShapeData(*(mvd[0]->_simple->_Fcst_sd));
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         if (f_simple_result.get(x, y)) {
            f_simple_sd.data.put(1.0, x, y);
         } else {
            f_simple_sd.data.put(bad_data_double, x, y);
         }
      }
   }
   ShapeData o_simple_sd = ShapeData(*(mvd[0]->_simple->_Fcst_sd));
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         if (o_simple_result.get(x, y)) {
            o_simple_sd.data.put(1.0, x, y);
         } else {
            o_simple_sd.data.put(bad_data_double, x, y);
         }
      }
   }

   ShapeData f_merge_sd = ShapeData(*(mvd[0]->_simple->_Fcst_sd));
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         if (f_merge_result.get(x, y)) {
            f_merge_sd.data.put(1.0, x, y);
         } else {
            f_merge_sd.data.put(bad_data_double, x, y);
         }
      }
   }
   
   int n_f_shapes;
   ShapeData f_merge_sd_split = split(f_merge_sd, n_f_shapes);
   string s = "Forecast Merge";
   _debug_shape_examine(s, f_merge_sd_split, nx, ny);

   ShapeData o_merge_sd = ShapeData(*(mvd[0]->_simple->_Fcst_sd));
   for (int x=0; x<nx; ++x) {
      for (int y=0; y<ny; ++y) {
         if (o_merge_result.get(x, y)) {
            o_merge_sd.data.put(1.0, x, y);
         } else {
            o_merge_sd.data.put(bad_data_double, x, y);
         }
      }
   }

   int n_o_shapes;
   ShapeData o_merge_sd_split = split(o_merge_sd, n_o_shapes);
   s = "Obs Merge";
   _debug_shape_examine(s, o_merge_sd_split, nx, ny);


   bool superobj_mode = true;  // set this false if an individual input has intensity set TRUE

   //
   // Filter the data to within the superobjects only and do statistics by invoking mode algorithm again
   // on the masked data, for all inputs that have intensity=TRUE
   //
 
   for (j=0; j<n_files; ++j) {

      if (!config.multivar_intensity[j]) {
         mlog << Debug(2) << "\n SKIPPING field " << j << " for masked intensities \n";
         continue;
      }

      superobj_mode = false;

      mlog << Debug(2) 
           << "\n starting masked data mode run " << (j + 1) << " of " << n_files
           << "\n" << sep << "\n";

      create_singlevar_intensity_objects(j, f_simple_result, o_simple_result, nx, ny, dir, *mvd[j],
                                         has_union, f_merge_sd_split, o_merge_sd_split);

      delete mvd[j];
      mvd[j] = 0;
   }
 
   mlog << Debug(2) << "\n finished with individual masked mode runs " << "\n" << sep << "\n";

   if (!superobj_mode) {

      // when an individual field was processed above, no super object step
      return 0;

   }
   

   // here run one more time using superobjects as input

   process_superobjects(f_simple_sd, o_simple_sd, f_merge_sd_split, o_merge_sd_split,
                        nx, ny, dir, ftype, otype, grid, has_union);

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
                                 BoolCalc &f_calc, BoolCalc &o_calc, int &n_files)
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

   //
   // make sure there are the same number of obs and fcst files
   //

   fcst_filenames = parse_ascii_file_list(fcst_fof.c_str());
   obs_filenames = parse_ascii_file_list(obs_fof.c_str());

   if ( fcst_filenames.n() != obs_filenames.n() )  {

      mlog << Error << "\n" << program_name
           << ": number of fcst and obs files should be the same in multivar mode!\n\n";
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
   if (num_multivar == 0) {

      // note this will not happen until the method
      // ModeConfInfo::check_multivar_not_immplemented() allows 'all false'
      // right now error exit before you get to here.
      
      mlog << Warning << "\nmultivar_frontend() -> "
           << "empty multivar intensity array, setting to all FALSE \n\n";

      for (int i=0; i<n_files; ++i) {
         config.multivar_intensity.add(false);
      }

   } else if (num_multivar != n_files) {

      mlog << Error << "\n" << program_name 
           << ": wrong size multivar_intensity array, wanted "
           << n_files << " got " << num_multivar << "\n\n";
      exit ( 1 );
   }

   // check that fcst and obs arrays are both the same length as everything else
   // actually don't need this as it's done inside the mode_config class

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

MultiVarData *create_simple_objects(int j, int n_files, const string &fcst_filename,
                                    const string &obs_filename,
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

   //
   //  run the pass1 portions of mode, which creates simple objects
   //

   mlog << Debug(3) << "Running mode command: \"" << command << "\"\n\n";
   ModeFrontEnd *frontend = new ModeFrontEnd;
   int status = frontend->run(mode_argv, ModeFrontEnd::MULTIVAR_PASS1, j, n_files);
   MultiVarData *mvdi = frontend->get_multivar_data();
   delete frontend;

   //
   //  run the pass1 merge portions of mode, which creates merge simple objects
   //

   frontend = new ModeFrontEnd;
   status = frontend->run(mode_argv, ModeFrontEnd::MULTIVAR_PASS1_MERGE, j, n_files);
   frontend->addMultivarMergePass1(mvdi);
   delete frontend;

   return mvdi;
}

////////////////////////////////////////////////////////////////////////

void create_singlevar_intensity_objects(int j, const BoolPlane &f_result,
                                        const BoolPlane &o_result, int nx, int ny,
                                        const ConcatString &dir, MultiVarData &mvd,
                                        bool has_union, ShapeData &f_merge, ShapeData &o_merge)
{
   // mask the input data to be valid only inside the simple super objects
   
   mask_data("Fcst", nx, ny, f_result, mvd._simple->_Fcst_sd->data);
   mask_data("Obs", nx, ny, o_result, mvd._simple->_Obs_sd->data);

   StringArray mode_argv;
   char junk [256];

   //
   //  build the command for running mode
   //
   mode_argv.clear();
   mode_argv.add(mode_path);
   mode_argv.add(config_file);
   mode_argv.add("-v");
   snprintf(junk, sizeof(junk), "%d", mlog.verbosity_level());
   mode_argv.add(junk);
   mode_argv.add("-outdir");
   mode_argv.add(dir);

   mlog << Debug(1) << "Running filtered mode \n\n";

   ModeFrontEnd *frontend = new ModeFrontEnd;
   int status = frontend->run_multivar_pass2(mode_argv, mvd, has_union, f_merge, o_merge, j);
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
   //  build the command for running mode
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
