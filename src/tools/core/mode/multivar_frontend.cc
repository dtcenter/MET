// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


using namespace std;

#include "multivar_frontend.h"

#include "mode_usage.h"

#ifdef WITH_PYTHON
#include "global_python.h"
#endif

////////////////////////////////////////////////////////////////////////


extern const char * const program_name;

static const char sep [] = "====================================================";

static string outdir;
static int compress_level = -1;

// for multivar mode, this is the default file
static const char mode_default_config [] = "MET_BASE/config/MODEMultivarConfig_default";

static const int dir_creation_mode = 0755;       

static ModeExecutive *mode_exec = 0;


////////////////////////////////////////////////////////////////////////

MultivarFrontEnd::MultivarFrontEnd()
{
   // this is hardwired for the multivar case, at least for now
   do_clusters = false;
   default_out_dir = ".";
   compress_level = -1;
   mode_exec = 0;
}

////////////////////////////////////////////////////////////////////////

int MultivarFrontEnd::run(const StringArray & Argv)

{

   // initialize

   init(Argv);

   mlog << Debug(2) << "\n" << sep << "\n";

   // read in all the data

   // in the conf object, shift *can* be set independently for obs and fcst
   int shift = config.shift_right;

   for (int i=0; i<n_fcst_files; ++i) {
      GrdFileType ft, ot;
      ft = config.file_type_for_field(true, i);
      ot = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_obs));
      read_input(fcst_filenames[i], i, ModeDataType_MvMode_Fcst, ft, ot, shift);

   }
   for (int i=0; i<n_obs_files; ++i) {
      GrdFileType ft, ot;
      ft = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_fcst));
      ot = config.file_type_for_field(false, i);
      read_input(obs_filenames[i], i, ModeDataType_MvMode_Obs, ot, ft, shift);
   }
   
   // double check some thing that are now set
   config.check_multivar_not_implemented();

   // need data to set percentile thresholds so do so now
   config.config_set_all_percentile_thresholds(fcstInput, obsInput);


   // Define the verification grid using the 0th fcst and obs inputs
   create_verif_grid();

   //
   // do the individual mode runs which produce everything needed to create
   // super objects (stored in 'mvdObs', 'mvdFcst') (both simple and merge steps
   // are done here).
   //

   for (int j=0; j<n_fcst_files; ++j)  {

      mlog << Debug(2) 
           << "\n" << sep << "\ncreating simple forecast objects from forecast "
           << (j + 1) << " of " << n_fcst_files << "\n" << sep << "\n";
      MultiVarData *mvdi = create_simple_objects(ModeDataType_MvMode_Fcst, j,
                                                 n_fcst_files, fcst_filenames[j],
                                                 fcstInput[j]);
      if (j > 0) {
         mvdFcst[0]->checkFileTypeConsistency(*mvdi, j);
      }
      mvdFcst.push_back(mvdi);
      mvdi->print();
   }   //  for j

   for (int j=0; j<n_obs_files; ++j)  {

      mlog << Debug(2) 
           << "\n" << sep << "\ncreating simple obs objects from obs "
           << (j + 1) << " of " << n_obs_files << "\n" << sep << "\n";
      MultiVarData *mvdi = create_simple_objects(ModeDataType_MvMode_Obs, j,
                                                 n_obs_files, obs_filenames[j],
                                                 obsInput[j]);
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
   // Filter the data to within the superobjects only and do statistics by invoking mode
   // algorithm again on the masked data pairs
   //

   for (int k=0; k<config.fcst_multivar_compare_index.n(); ++k)
   {
      int findex = config.fcst_multivar_compare_index[k] - 1;
      int oindex = config.obs_multivar_compare_index[k] - 1;

      create_intensity_comparisons(findex, oindex, fsuper, osuper,
                                   *mvdFcst[findex], *mvdObs[oindex],
                                   fcst_filenames[findex], obs_filenames[oindex]);
   }

   mlog << Debug(2) << "\n finished with multivar intensity comparisons \n" << sep << "\n";

   // special case of just superobject statistics, no comparisons configured

   if (config.fcst_multivar_compare_index.n() <= 0) {

      process_superobjects(fsuper, osuper, *mvdFcst[0], *mvdObs[0]); 
   }
   
   //
   //  done
   //
   return (0);
}

////////////////////////////////////////////////////////////////////////

MultivarFrontEnd::~MultivarFrontEnd()
{
   if ( mode_exec ) {
      delete mode_exec;  mode_exec = 0;
   }


   for (int j=0; j<n_fcst_files; ++j)  {
      delete mvdFcst[j];
      mvdFcst[j] = 0;
   }
   for (int j=0; j<n_obs_files; ++j)  {
      delete mvdObs[j];
      mvdObs[j] = 0;
   }
   
}


////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::init(const StringArray & Argv)
{
   int Argc = Argv.n();

   if ( Argc < 4 )  multivar_usage();

   // set some logging related things here, used in all further processing
   _process_command_line(Argv);

   // read the config as fully as possible without any data reads
   // (Initialize all the input fields)
   _read_config(config_file);

   // check for length discrepencies and set up input files
   _setup_inputs();

   // set output path
   _set_output_path();
}

////////////////////////////////////////////////////////////////////////


void MultivarFrontEnd::set_outdir(const StringArray & a)

{

   outdir = a[0];

   return;

}


////////////////////////////////////////////////////////////////////////


void MultivarFrontEnd::set_logfile(const StringArray & a)

{

   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);

   return;

}


////////////////////////////////////////////////////////////////////////


void MultivarFrontEnd::set_verbosity (const StringArray & a)

{

   mlog.set_verbosity_level(atoi(a[0].c_str()));

   return;

}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::set_compress(const StringArray & a)
{
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::read_input(const string &name, int index, ModeDataType type,
                                  GrdFileType f_t, GrdFileType other_t, int shift)
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
      f->data_plane(*(config.Fcst->var_info), dp);
      fcstInput.push_back(ModeInputData(name, dp, g, ft));
   } else {
      config.process_config_field(other_t, ft, type, index);
      f->data_plane(*(config.Obs->var_info), dp);
      obsInput.push_back(ModeInputData(name, dp, g, ft));
   }         
      
   delete f;
}
      
////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::create_verif_grid()
{
   mlog << Debug(2) << "\n creating the verification grid \n" << sep << "\n";

   _init_exec(ModeExecutive::TRADITIONAL, "None", "None");
   // mode_exec->init_multivar_verif_grid(fcstInput[0]._dataPlane,
   //                                     obsInput[0]._dataPlane, config);
   // ModeConfInfo & conf = mode_exec->engine.conf_info;
   // conf.set_field_index(0);
   mode_exec->setup_verification_grid(fcstInput[0], obsInput[0], config);
   verification_grid = mode_exec->grid;
   delete mode_exec;  mode_exec = 0;
}

////////////////////////////////////////////////////////////////////////

MultiVarData *MultivarFrontEnd::create_simple_objects(ModeDataType dtype,  int j,
                                                      int n_files,
                                                      const string &filename,
                                                      const ModeInputData &input)
{
   //
   // create simple non merged objects
   //
   _simple_objects(ModeExecutive::MULTIVAR_SIMPLE, dtype, j, n_files,
                   filename, input);
   MultiVarData *mvdi = mode_exec->get_multivar_data(dtype);
   delete mode_exec; mode_exec = 0;

   //
   // create simple merged objects
   //
   _simple_objects(ModeExecutive::MULTIVAR_SIMPLE_MERGE, dtype, j, n_files,
                   filename, input);
   mode_exec->add_multivar_merge_data(mvdi, dtype);
   delete mode_exec;  mode_exec = 0;
   return mvdi;
}

////////////////////////////////////////////////////////////////////////

void
MultivarFrontEnd::create_intensity_comparisons(int findex, int oindex,
                                               const ModeSuperObject &fsuper,
                                               const ModeSuperObject &osuper,
                                               MultiVarData &mvdf, MultiVarData &mvdo,
                                               const string &fcst_filename,
                                               const string &obs_filename)
{

   // mask the input data to be valid only inside the simple super objects
   fsuper.mask_data_simple("Fcst", mvdf);
   osuper.mask_data_simple("Obs", mvdo);

   mlog << Debug(1) << "Running mvmode intensity comparisions \n\n";

   _init_exec(ModeExecutive::MULTIVAR_INTENSITY, fcst_filename, obs_filename);
   mode_exec->init_multivar_intensities(mvdf._type, mvdo._type, config);

   ModeConfInfo & conf = mode_exec->engine.conf_info;
   conf.set_field_index(findex, oindex);

   // for multivar intensities, explicity set the level and units using stored values
   // from pass1
   conf.Fcst->var_info->set_level_name(mvdf._level.c_str());
   conf.Fcst->var_info->set_units(mvdf._units.c_str());
   if (fsuper._hasUnion && conf.Fcst->merge_flag == MergeType_Thresh) {
      mlog << Warning << "\nModeFrontEnd::multivar_intensity_comparisons() -> "
           << "Logic includes union '||' along with  'merge_flag=THRESH' "
           << ". This can lead to bad results\n\n";
   }
   conf.Obs->var_info->set_level_name(mvdo._level.c_str());
   conf.Obs->var_info->set_units(mvdo._units.c_str());
   if (osuper._hasUnion && conf.Obs->merge_flag == MergeType_Thresh) {
      mlog << Warning << "\nModeFrontEnd::multivar_intensity_comparisons() -> "
           << "Logic includes union '||' along with  'merge_flag=THRESH' "
           << ". This can lead to bad results\n\n";
   }
       
   //
   // set up data access using inputs
   //
   mode_exec->setup_multivar_fcst_obs_data_intensities(mvdf, mvdo);

   //
   // run the mode algorithm for multivar intensities
   //
   _intensity_compare_mode_algorithm(mvdf, mvdo, fsuper, osuper);

   delete mode_exec;  mode_exec = 0;
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::process_superobjects(ModeSuperObject &fsuper,
                                            ModeSuperObject &osuper,
                                            const MultiVarData &mvdf,
                                            const MultiVarData &mvdo)
{
   mlog << Debug(1) << "Running superobject mode \n\n";

   // set the data to 0 inside superobjects and missing everywhere else

   fsuper.mask_data_super("FcstSimple", mvdf);
   osuper.mask_data_super("ObsSimple", mvdo);

   _init_exec(ModeExecutive::MULTIVAR_SUPER, "None", "None");
   mode_exec->init_multivar_intensities(mvdf._type, mvdo._type, config);

   ModeConfInfo & conf = mode_exec->engine.conf_info;
   if ((fsuper._hasUnion || osuper._hasUnion) &&
       (conf.Fcst->merge_flag == MergeType_Thresh ||
        conf.Obs->merge_flag == MergeType_Thresh)) {
      mlog << Warning << "\nModeFrontEnd::run_super() -> "
           << "Logic includes union '||' along with  'merge_flag=THRESH' "
           << ". This can lead to bad results\n\n";
   }
       
   //
   // set up data access using inputs
   //
   mode_exec->setup_multivar_fcst_obs_data_super(fsuper._simple_sd, osuper._simple_sd,
                                                 *mvdf._grid);

   // run the mode algorithm
   _superobject_mode_algorithm(fsuper, osuper);

   delete mode_exec;  mode_exec = 0;
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_process_command_line(const StringArray & argv)

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


void MultivarFrontEnd::_read_config(const string & filename)

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

void MultivarFrontEnd::_setup_inputs()
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

void MultivarFrontEnd::_set_output_path()
{
   int status;

   output_path.clear();

   // no longer want numbered subdirectories
   if ( outdir.length() > 0 )  output_path << outdir;

   //
   //  test to see of the output directory for this
   //    mode runs exists, and if not, create it
   //

   if ( ! directory_exists(output_path.c_str()) )  {

      mlog << Debug(2)
           << program_name << ": creating output directory \""
           << output_path << "\"\n\n";

      status = _mkdir(output_path.c_str());

      if ( status < 0 )  {

         mlog << Error << "\nset_multivar_dir() ->"
              << " unable to create output directory \""
              << output_path << "\"\n\n";

         exit ( 1 );
      }
   }
}

////////////////////////////////////////////////////////////////////////

int MultivarFrontEnd::_mkdir(const char *dir)
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

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_simple_objects(ModeExecutive::Processing_t p,
                                       ModeDataType dtype,
                                       int j, int n_files, const string &filename,
                                       const ModeInputData &input)
{
   if (dtype == ModeDataType_MvMode_Fcst) {
      _init_exec(p, filename, "None");
      mode_exec->init_multivar_simple(j, n_files, dtype, config);
      mode_exec->setup_multivar_fcst_data(verification_grid, input);
   } else {
      _init_exec(p, "None", filename);
      mode_exec->init_multivar_simple(j, n_files, dtype, config);
      mode_exec->setup_multivar_obs_data(verification_grid, input);
   }
   
   _simple_mode_algorithm(p);
}   

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_init_exec(ModeExecutive::Processing_t p,
                                  const string &ffile,
                                  const string &ofile)
{
   mlog << Debug(1) << "Running multivar front end for " << ModeExecutive::stype(p) << "\n";

   if ( mode_exec )  { delete mode_exec;  mode_exec = 0; }

   mode_exec = new ModeExecutive();
   // compress_level = -1;
   mode_exec->fcst_file = ffile;
   mode_exec->obs_file = ofile;
   mode_exec->match_config_file = config_file;
   mode_exec->out_dir = output_path;
}

////////////////////////////////////////////////////////////////////////

void
MultivarFrontEnd::_superobject_mode_algorithm(const ModeSuperObject &fsuper,
                                              const ModeSuperObject &osuper)
{
   _mode_algorithm_init();
   mode_exec->clear_internal_r_index();
   mode_exec->do_conv_thresh_multivar_super();
   mode_exec->do_match_merge_multivar(fsuper._merge_sd_split, osuper._merge_sd_split,
                                      ModeExecutive::MULTIVAR_SUPER);
   mode_exec->process_output_multivar_super();
   mode_exec->clear_internal_r_index();
#ifdef  WITH_PYTHON
    GP.finalize();
 #endif
}

////////////////////////////////////////////////////////////////////////

void
MultivarFrontEnd::_intensity_compare_mode_algorithm(const MultiVarData &mvdf,
                                                    const MultiVarData &mvdo,
                                                    const ModeSuperObject &fsuper,
                                                    const ModeSuperObject &osuper)
{
   _mode_algorithm_init();
   mode_exec->do_conv_thresh_multivar_intensity_compare();
   mode_exec->do_match_merge_multivar(fsuper._merge_sd_split, osuper._merge_sd_split,
                                      ModeExecutive::MULTIVAR_INTENSITY);
      // here replace raw data and min/max for plotting
   mode_exec->process_output_multivar_intensity_compare(&mvdf, &mvdo);
   mode_exec->clear_internal_r_index();
#ifdef  WITH_PYTHON
    GP.finalize();
 #endif
}                                     


////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_simple_mode_algorithm(ModeExecutive::Processing_t p)
{
   _mode_algorithm_init();
   mode_exec->clear_internal_r_index();
   mode_exec->do_conv_thresh_multivar_simple(p);
   mode_exec->clear_internal_r_index();
   
#ifdef  WITH_PYTHON
    GP.finalize();
 #endif
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_mode_algorithm_init() const
{
   const ModeConfInfo & conf = mode_exec->engine.conf_info;
   if ( conf.quilt )  {
      mlog << Error << "\nMultiVarFontend::mode_algorithm() -> "
           << "quilting not yet implemented for multivar mode \n\n";
      exit ( 1 );
   }

   int NCT = conf.n_conv_threshs();
   int NCR = conf.n_conv_radii();

   if ( NCT != NCR )  {

      mlog << Error << "\nMultivarFrontEnd::_mode_algorithm_init() ->"
           << "all convolution radius and threshold arrays must have the same number of elements\n\n";

      exit ( 1 );

   }

   if (NCT > 1) {

      mlog << Error << "\nMultivarFrontEnd::_mode_algorithm_init() ->"
           << ": multiple convolution radii and thresholds not implemented in multivar mode\n\n";

      exit ( 1 );
   }
}



    
