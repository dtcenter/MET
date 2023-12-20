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
#include "mode_input_data.h"
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
static int compress_level = -1;

static Grid verification_grid;

////////////////////////////////////////////////////////////////////////


static void set_outdir    (const StringArray &);
static void set_logfile   (const StringArray &);
static void set_verbosity (const StringArray &);
static void set_compress  (const StringArray &);

static void read_input(const string &name, int index, ModeDataType type,
                       GrdFileType f_t, GrdFileType other_t, int shift,
                       vector<ModeInputData> &inputs);

static void multivar_consistency_checks(StringArray &fcst_filenames, StringArray &obs_filenames,
                                        BoolCalc &f_calc, BoolCalc &o_calc, int &n_fcst_files,
                                        int &n_obs_files);

static ConcatString set_multivar_dir();

static void create_verification_grid(const ModeInputData &fcst, const ModeInputData &obs);


static MultiVarData *create_simple_objects(ModeDataType dtype, int j, int n_files,
                                           const string &filename, 
                                           const ConcatString &dir,
                                           const ModeInputData &input);

static void create_intensity_comparisons(int findex, int oindex,
                                         const ModeSuperObject &fsuper,
                                         const ModeSuperObject &osuper,
                                         const ConcatString &dir,
                                         MultiVarData &mvdf, MultiVarData &mvdo,
                                         bool has_union_f, bool has_union_o,
                                         const string &fcst_filename,
                                         const string &obs_filename);

static void process_superobjects(ModeSuperObject &fsuper,
                                 ModeSuperObject &osuper,
                                 int nx, int ny, const ConcatString &dir,
                                 GrdFileType ftype, GrdFileType otype, const Grid &grid,
                                 bool has_union);

static void mask_data(const string &name, int nx, int ny, const BoolPlane &mask,
                      DataPlane &data);
static void mask_data_super(const string &name, int nx, int ny, DataPlane &data);

static void read_config(const string & filename);

static void process_command_line(const StringArray &);

static int  _mkdir(const char *dir);

////////////////////////////////////////////////////////////////////////


int multivar_frontend(const StringArray & Argv)

{

   const int Argc = Argv.n();

   if ( Argc < 4 )  multivar_usage();

   int j, n_fcst_files, n_obs_files;
   StringArray fcst_filenames;
   StringArray  obs_filenames;
   BoolCalc f_calc, o_calc ;

   // set some logging related things here, used in all further processing

   process_command_line(Argv);

   // read the config as fully as possible without any data reads
   // (Initialize all the input fields)
   
   read_config(config_file);

   // check for length discrepencies.

   multivar_consistency_checks(fcst_filenames, obs_filenames, f_calc, o_calc,
                               n_fcst_files, n_obs_files);

   mlog << Debug(2) << "\n" << sep << "\n";

   ConcatString dir = set_multivar_dir();

   // read in all the data
   vector<ModeInputData> fcstInput, obsInput;
   GrdFileType ft, ot;


   // in the conf object, shift *can* be set independently for obs and fcst
   int shift = config.shift_right;

   for (int i=0; i<n_fcst_files; ++i) {
      ft = config.file_type_for_field(true, i);
      ot = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_obs));
      read_input(fcst_filenames[i], i, ModeDataType_MvMode_Fcst, ft, ot, shift,
                 fcstInput);
   }
   for (int i=0; i<n_obs_files; ++i) {
      ft = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_fcst));
      ot = config.file_type_for_field(false, i);
      read_input(obs_filenames[i], i, ModeDataType_MvMode_Obs, ot, ft, shift,
                 obsInput);
   }
   

   config.check_multivar_not_implemented();
   config.config_set_all_percentile_thresholds(fcstInput, obsInput);


   // first thing to do is to define the verification grid using the 0th
   // fcst and obs inputs
   mlog << Debug(2) << "\n creating the verification grid \n" << sep << "\n";

   create_verification_grid(fcstInput[0], obsInput[0]);

   //
   // do the individual mode runs which produce everything needed to create
   // super objects (stored in 'mvdObs', 'mvdFcst') (both simple and merge steps
   // are done here).
   //

   vector<MultiVarData *> mvdObs, mvdFcst;

   for (j=0; j<n_fcst_files; ++j)  {

      mlog << Debug(2) 
           << "\n" << sep << "\ncreating simple forecast objects from forecast " << (j + 1) << " of " << n_fcst_files << "\n" << sep << "\n";
      MultiVarData *mvdi = create_simple_objects(ModeDataType_MvMode_Fcst, j, n_fcst_files, fcst_filenames[j], dir, fcstInput[j]);
      if (j > 0) {
         mvdFcst[0]->checkFileTypeConsistency(*mvdi, j);
      }
      mvdFcst.push_back(mvdi);
      mvdi->print();
   }   //  for j

   for (j=0; j<n_obs_files; ++j)  {

      mlog << Debug(2) 
           << "\n" << sep << "\ncreating simple obs objects from obs " << (j + 1) << " of " << n_obs_files << "\n" << sep << "\n";
      MultiVarData *mvdi = create_simple_objects(ModeDataType_MvMode_Obs, j, n_obs_files, obs_filenames[j], dir, obsInput[j]);
      if (j > 0) {
         mvdObs[0]->checkFileTypeConsistency(*mvdi, j);
      }
      mvdObs.push_back(mvdi);
      mvdi->print();
   }   //  for j

   mlog << Debug(2) << "\n finished with simple multivar mode runs " << "\n" << sep << "\n";

   // now create forecast and obs superobjects
   
   ModeSuperObject fsuper(true, n_fcst_files, do_clusters, mvdFcst, f_calc);
   ModeSuperObject osuper(true, n_obs_files, do_clusters, mvdObs, o_calc);
                          
   //
   // Filter the data to within the superobjects only and do statistics by invoking mode algorithm again
   // on the masked data pairs
   //
   bool f_has_union = f_calc.has_union();
   bool o_has_union = o_calc.has_union();

   for (int k=0; k<config.fcst_multivar_compare_index.n(); ++k)
   {
      int findex = config.fcst_multivar_compare_index[k] - 1;
      int oindex = config.obs_multivar_compare_index[k] - 1;

      create_intensity_comparisons(findex, oindex, fsuper, osuper,
                                   dir, *mvdFcst[findex], *mvdObs[oindex],
                                   f_has_union, o_has_union,
                                   fcst_filenames[findex], obs_filenames[oindex]);
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

      process_superobjects(fsuper, osuper, nx, ny, dir, ftype, otype, grid, has_union);
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

void set_compress(const StringArray & a)
{
   compress_level = atoi(a[0].c_str());
}



////////////////////////////////////////////////////////////////////////


void read_config(const string & filename)

{

   ConcatString path;

   path = replace_path(mode_default_config);

   config.read_config(path.c_str(), filename.c_str());

   // process the config except for the fields
   config.process_config_except_fields();

   // done once here, used for all data
   // what is this, command line overrides config?  look deeper.. remove from exec
   // except traditional mode
   if (compress_level >= 0) config.nc_info.set_compress_level(compress_level);
   // from within mode_exec:
   // engine.conf_info.nc_info.compress_level = engine.conf_info.get_compression_level();


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
   cline.add(set_compress, "-compress", 1);

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

void read_input(const string &name, int index, ModeDataType type,
                GrdFileType f_t, GrdFileType other_t, int shift,
                vector<ModeInputData> &inputs)
   {
      Met2dDataFileFactory mtddf_factory;
      Met2dDataFile *f = mtddf_factory.new_met_2d_data_file(name.c_str(), f_t);
      if (!f) {
         mlog << Error << "\nTrouble reading fcst file \""
              << name << "\"\n\n";
         exit(1);
      }
      Grid g = f->grid();
      GrdFileType ft = f->file_type();

      //?
      f->set_shift_right(shift);

      // update config now that we know file type (this sets Fcst to index i)
      DataPlane dp;

      if (type == ModeDataType_MvMode_Fcst) {
         config.process_config_field(ft, other_t, type, index);
         // need to have set the var_info!
         f->data_plane(*(config.Fcst->var_info), dp);
      } else {
         config.process_config_field(other_t, ft, type, index);
         // need to have set the var_info!
         f->data_plane(*(config.Obs->var_info), dp);
      }         
      
      inputs.push_back(ModeInputData(name, dp, g, ft));
      delete f;
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

      mlog << Error << "\nmultivar_consistency_checks() ->"
           << "fcst multivar logic not specified in multivar mode!\n\n";
      exit ( 1 );

   }

   if ( config.obs_multivar_logic.empty() )  {

      mlog << Error << "\nmultivar_consistency_checks() ->"
           << "obs multivar logic not specified in multivar mode!\n\n";
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

      mlog << Error << "\nmultivar_consistency_checks() ->"
           << "Want at least 2 input files for fcst or obs in multivar mode, neither had 2 or more\n\n";
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
      mlog << Error << "\nmultivar_consistency_checks() ->"
           << " Need equal number of multivar_compare_index entries for obs and fcst\n\n";
      exit(1);
   }

   bool badIndex = false;
   for (int k=0; k<config.fcst_multivar_compare_index.n(); ++k)
   {
      int findex = config.fcst_multivar_compare_index[k];
      int oindex = config.obs_multivar_compare_index[k];
      if (findex <= 0 || findex > n_fcst_files) {
         mlog << Error << "\nmultivar_consistency_checks() ->"
              << " forecast index " << findex
              << " out of range, " << conf_key_fcst_multivar_compare_index << " array\n";
         badIndex = true;
      }
      if (oindex <= 0 || oindex > n_obs_files) {
         mlog << Error << "\nmultivar_consistency_checks() ->"
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

         mlog << Error << "\nset_multivar_dir() ->"
              << " unable to create output directory \""
              << dir << "\"\n\n";

         exit ( 1 );
      }
   }
   return dir;
}

////////////////////////////////////////////////////////////////////////

void create_verification_grid(const ModeInputData &fcst, 
                              const ModeInputData &obs)
{
   ModeFrontEnd *frontend = new ModeFrontEnd;
   verification_grid = frontend->create_verification_grid(fcst, obs,
                                                          config_file,
                                                          config);
   delete frontend;
}

////////////////////////////////////////////////////////////////////////

MultiVarData *create_simple_objects(ModeDataType dtype,  int j, int n_files,
                                    const string &filename,
                                    const ConcatString &dir,
                                    const ModeInputData &input)
{
   ModeFrontEnd *frontend = new ModeFrontEnd;
   int status =
      frontend->create_multivar_simple_objects(config, dtype, verification_grid, input, 
                                               filename, config_file, dir,
                                               j, n_files);
   MultiVarData *mvdi = frontend->get_multivar_data(dtype);
   delete frontend;

   //
   //  create simple merge objects
   //

   frontend = new ModeFrontEnd;
   status = frontend->create_multivar_merge_objects(config, dtype, verification_grid, input,
                                                    filename, config_file, dir, 
                                                    j, n_files);

   // add the merge results to the mvdi object
   frontend->add_multivar_merge_data(mvdi, dtype);
   delete frontend;

   return mvdi;
}

////////////////////////////////////////////////////////////////////////

void
create_intensity_comparisons(int findex, int oindex,
                             const ModeSuperObject &fsuper,
                             const ModeSuperObject &osuper,
                             const ConcatString &dir,
                             MultiVarData &mvdf, MultiVarData &mvdo,
                             bool has_union_f, bool has_union_o,
                             const string &fcst_filename,
                             const string &obs_filename)
{

   // mask the input data to be valid only inside the simple super objects
   int nx = mvdf._nx;
   int ny = mvdf._ny;
   
   mask_data("Fcst", nx, ny, fsuper._simple_result, mvdf._simple->_sd->data);
   mask_data("Obs", nx, ny, osuper._simple_result, mvdo._simple->_sd->data);

   mlog << Debug(1) << "Running mvmode intensity comparisions \n\n";

   ModeFrontEnd *frontend = new ModeFrontEnd;
   int status = frontend->multivar_intensity_comparisons(config, mvdf, mvdo, has_union_f,
                                                         has_union_o,
                                                         fsuper._merge_sd_split,
                                                         osuper._merge_sd_split,
                                                         findex,
                                                         oindex,
                                                         fcst_filename,
                                                         obs_filename,
                                                         config_file, dir);
   delete frontend;
}

////////////////////////////////////////////////////////////////////////

void process_superobjects(ModeSuperObject &fsuper,
                          ModeSuperObject &osuper,
                          int nx, int ny, const ConcatString &dir,
                          GrdFileType ftype, GrdFileType otype,
                          const Grid &grid, bool has_union)
{
   mlog << Debug(1) << "Running superobject mode \n\n";

   // set the data to 0 inside superobjects and missing everywhere else

   mask_data_super("FcstSimple", nx, ny, fsuper._simple_sd.data);
   mask_data_super("ObsSimple", nx, ny, osuper._simple_sd.data);
   

   ModeFrontEnd *frontend = new ModeFrontEnd;

   int status = frontend->run_super(config, fsuper, osuper,
                                    ftype, otype, grid, has_union,
                                    config_file, dir);
   delete frontend;
}
 
////////////////////////////////////////////////////////////////////////

void mask_data(const string &name, int nx, int ny, const BoolPlane &bp, DataPlane &data)
{

   if (nx != data.nx() || ny != data.ny()) {
      mlog << Error << "\nmask_data() -> " << name 
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
      mlog << Error << "\nmask_data_super() -> " << name 
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
               mlog << Error << "\n_mkdir() -> Error making " << tmp << "\n";
               return -1;
            }
         }
         *p = '/';
      }

   return (mkdir(tmp, dir_creation_mode));
}
