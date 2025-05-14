// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include "multivar_frontend.h"
#include "mode_usage.h"

#ifdef WITH_PYTHON
#include "global_python.h"
#endif

using namespace std;

////////////////////////////////////////////////////////////////////////

extern const char * const program_name;

static const char sep [] = "====================================================";

static string outdir;
static int compress_level = -1;

// for multivar mode, this is the default file
static const char mode_default_config [] = "MET_BASE/config/MODEMultivarConfig_default";

static const int dir_creation_mode = 0755;       

static ModeExecutive *mode_exec = nullptr;

////////////////////////////////////////////////////////////////////////

MultivarFrontEnd::MultivarFrontEnd()
{
   // this is hardwired for the multivar case, at least for now
   do_clusters = false;
   default_out_dir = ".";
   compress_level = -1;
   mode_exec = nullptr;
}

////////////////////////////////////////////////////////////////////////

MultivarFrontEnd::~MultivarFrontEnd()
{
   if ( mode_exec ) {
      delete mode_exec;  mode_exec = nullptr;
   }
 }

////////////////////////////////////////////////////////////////////////

int MultivarFrontEnd::run(const StringArray & Argv)
{
   // initialize

   _init(Argv);

   mlog << Debug(1) << "\n" << sep << "\n";

   // in the conf object, shift *can* be set independently for obs and fcst
   int shift = config.shift_right;

   // read in all the data
   for (int i=0; i<n_fcst_files; ++i) {
      GrdFileType ft = config.file_type_for_field(true, i);
      GrdFileType ot = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_obs));
      _read_input(fcst_filenames[i], i, ModeDataType::MvMode_Fcst, ft, ot, shift);

   }
   for (int i=0; i<n_obs_files; ++i) {
      GrdFileType ft = parse_conf_file_type(config.conf.lookup_dictionary(conf_key_fcst));
      GrdFileType ot = config.file_type_for_field(false, i);
      _read_input(obs_filenames[i], i, ModeDataType::MvMode_Obs, ot, ft, shift);
   }
   
   // double check some thing that are now set
   config.check_multivar_not_implemented();

   // need data to set percentile thresholds so do so now
   config.config_set_all_percentile_thresholds(fcstInput, obsInput);

   // Define the verification grid using the 0th fcst and obs inputs
   _create_verif_grid();

   // in the implementation now, all 4 of these numbers must be the same without quilting
   // and NCTF must equal NCTO, NCRF must equal NCRO with quilting
   int NCTF = config.n_conv_threshs_fcst();
   int NCRF = config.n_conv_radii_fcst();
   int NCTO = config.n_conv_threshs_obs();
   int NCRO = config.n_conv_radii_obs();

   if (NCTF != NCTO) {
      mlog << Error << "\nMultivarFrontEnd::run() -> "
           << "all convolution threshold arrays must have the same number of elements\n\n";
      exit ( 1 );
   }
   if (NCRF != NCRO) {
      mlog << Error << "\nMultivarFrontEnd::run() -> "
           << "all convolution radius arrays must have the same number of elements\n\n";
      exit ( 1 );
   }

   if ((!config.quilt) && (NCTF != NCRF || NCTO != NCRO)) {
      mlog << Error << "\nMultivarFrontEnd::run() -> "
           << "all convolution radius and threshold arrays must have the same number of elements without quilting\n\n";
      exit ( 1 );
   }

   // the numbers to use
   int NT = NCTF;
   int NR = NCRF;
   
   // containers for information needed to do additional things with simple objects after creation
   vector<SimpleObjects> fcstSimple;
   vector<SimpleObjects> obsSimple;

   // create simple objects for all convolution settings
   if (config.quilt) {
      for (int ir=0; ir<NR; ++ir) {
         for (int it=0; it<NT; ++it) {
            SimpleObjects OF;
            SimpleObjects OO;
            _create_simple_objects(ModeDataType::MvMode_Fcst, "forecast", ir, it, n_fcst_files,
                                   fcst_filenames, fcstInput, f_calc, OF);
            fcstSimple.emplace_back(OF);
            _create_simple_objects(ModeDataType::MvMode_Obs, "obs", ir, it, n_obs_files,
                                   obs_filenames, obsInput, o_calc, OO);
            obsSimple.emplace_back(OO);
         }
      }
   }
   else {
      for (int ir=0; ir<NR; ++ir) {
         SimpleObjects OF;
         SimpleObjects OO;
         _create_simple_objects(ModeDataType::MvMode_Fcst, "forecast", ir, ir, n_fcst_files,
                                fcst_filenames, fcstInput, f_calc, OF);
         fcstSimple.emplace_back(OF);
         _create_simple_objects(ModeDataType::MvMode_Obs, "obs", ir, ir, n_obs_files,
                                obs_filenames, obsInput, o_calc, OO);
         obsSimple.emplace_back(OO);
      }
   }

   // Note at this point we know the compare index arrays are the same length for fcst
   // and obs

   for (size_t fi=0; fi<fcstSimple.size(); ++fi) {
      auto oi = (int) fi;

      // Filter the data to within the superobjects only and do statistics by invoking mode
      // algorithm again on the masked data pairs

      for (int k=0; k<config.fcst_multivar_compare_index.n(); ++k)
      {
         int findex = config.fcst_multivar_compare_index[k] - 1;
         int oindex = config.obs_multivar_compare_index[k] - 1;

         _create_intensity_comparisons(fcstSimple[fi], findex, obsSimple[oi], oindex, 
                                       fcst_filenames[findex], obs_filenames[oindex]);
      }

      // special case of just superobject statistics, no comparisons configured

      if (config.fcst_multivar_compare_index.n() <= 0) {
         _process_superobjects(fcstSimple[fi], obsSimple[oi]);
      }
   }
   
   mlog << Debug(1) << "\n finished with multivar intensity comparisons \n" << sep << "\n";

   // clear out memory stored in the simple objects

   for (auto &x : fcstSimple) x.clear();
   for (auto &x : obsSimple) x.clear();

   //
   //  done
   //
   return 0;
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

void MultivarFrontEnd::_init(const StringArray & Argv)
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

   return;
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_setup_inputs()
{
   //
   //  make sure the multivar logic programs are in the config file
   //

   if ( config.fcst_multivar_logic.empty() )  {

      mlog << Error << "\nMultivarFrontEnd::_setup_inputs() -> "
           << "fcst multivar logic not specified in multivar mode!\n\n";
      exit ( 1 );

   }

   if ( config.obs_multivar_logic.empty() )  {

      mlog << Error << "\nMultivarFrontEnd::_setup_inputs() -> "
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

      mlog << Error << "\nMultivarFrontEnd::_setup_inputs() -> "
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
      mlog << Error << "\nMultivarFrontEnd::_setup_inputs() -> "
           << "Need equal number of multivar_compare_index entries for obs and fcst\n\n";
      exit(1);
   }

   bool badIndex = false;
   for (int k=0; k<config.fcst_multivar_compare_index.n(); ++k)
   {
      int findex = config.fcst_multivar_compare_index[k];
      int oindex = config.obs_multivar_compare_index[k];
      if (findex <= 0 || findex > n_fcst_files) {
         mlog << Error << "\nMultivarFrontEnd::_setup_inputs() -> "
              << "forecast index " << findex
              << " out of range, " << conf_key_fcst_multivar_compare_index << " array\n";
         badIndex = true;
      }
      if (oindex <= 0 || oindex > n_obs_files) {
         mlog << Error << "\nMultivarFrontEnd::_setup_inputs() -> "
              << "obs index " << oindex
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

         mlog << Error << "\nMultivarFrontEnd::_set_output_path() -> "
              << "unable to create output directory \""
              << output_path << "\"\n\n";

         exit ( 1 );
      }
   }
}

////////////////////////////////////////////////////////////////////////

int MultivarFrontEnd::_mkdir(const char *dir) const
{
   char tmp[256];
   size_t len;

   snprintf(tmp, sizeof(tmp),"%s",dir);
   len = strlen(tmp);
   if (tmp[len - 1] == '/')
      tmp[len - 1] = 0;
   for (char *p = tmp + 1; *p; p++)
      if (*p == '/') {
         *p = 0;
         string s = tmp;
         if (s != "." &&
             mkdir(tmp, dir_creation_mode) < 0) {
            mlog << Error << "\nMultivarFrontEnd::_mkdir() -> "
                 << "Error making " << tmp << "\n";
            return -1;
         }
         *p = '/';
      }

   return mkdir(tmp, dir_creation_mode);
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_read_input(
                          const string &name, int index,
                          ModeDataType type, GrdFileType f_t,
                          GrdFileType other_t, int shift)
{
   Met2dDataFileFactory mtddf_factory;
   Met2dDataFile *f = mtddf_factory.new_met_2d_data_file(name.c_str(), f_t);
   if (!f) {
      mlog << Error << "\nMultivarFrontEnd::_read_input() -> "
           << "Trouble reading fcst file \"" << name << "\"\n\n";
      exit(1);
   }
   GrdFileType ft = f->file_type();

   // store shift right setting
   f->set_shift_right(shift);

   // update config now that we know file type (this sets Fcst to index i)
   DataPlane dp;
   Grid g;

   if (type == ModeDataType::MvMode_Fcst) {
      config.process_config_field(ft, other_t, type, index);
      f->data_plane(*(config.Fcst->var_info), dp);
      if(g.nxy() == 0) g = f->grid();
      fcstInput.emplace_back(ModeInputData(name, dp, g));
   } else {
      config.process_config_field(other_t, ft, type, index);
      f->data_plane(*(config.Obs->var_info), dp);
      if(g.nxy() == 0) g = f->grid();
      obsInput.emplace_back(ModeInputData(name, dp, g));
   }         
      
   delete f;
}
      
////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_create_verif_grid()
{
   mlog << Debug(1) << "\n creating the verification grid \n" << sep << "\n";

   _init_exec(ModeExecutive::TRADITIONAL, "None", "None");
   mode_exec->setup_verification_grid(fcstInput[0], obsInput[0], config);
   verification_grid = mode_exec->grid;
   delete mode_exec;  mode_exec = nullptr;
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_create_simple_objects(
                          ModeDataType dtype, const std::string &name,
                          int rIndex, int tIndex, int n_files,
                          const StringArray &filenames,
                          const std::vector<ModeInputData> &input,
                          BoolCalc &calc, SimpleObjects &O) const
{
   O.init(dtype, rIndex, tIndex);
   for (int j=0; j<n_files; ++j)  {
      mlog << Debug(1) 
           << "\n" << sep << "\ncreating simple " << name << " objects from " << name << " "
           << (j + 1) << " of " << n_files << " conv_radius[" << rIndex+1 << "] conv_thresh["
           << tIndex+1 << "]\n" << sep << "\n";
      MultiVarData *mvdi = _create_simple_multivar_data(dtype, rIndex, tIndex, j, n_files, 
                                                        filenames[j], input[j]);
      mvdi->print();
      O._mvd.emplace_back(mvdi);
   }
   O.setSuper(dtype == ModeDataType::MvMode_Fcst, n_files, do_clusters, calc);
}

////////////////////////////////////////////////////////////////////////

MultiVarData *MultivarFrontEnd::_create_simple_multivar_data(
                                   ModeDataType dtype,
                                   int rIndex, int tIndex,
                                   int j, int n_files,
                                   const string &filename,
                                   const ModeInputData &input) const
{
   //
   // create simple non merged objects
   //
   _simple_objects(ModeExecutive::MULTIVAR_SIMPLE, dtype, rIndex, tIndex, j, n_files,
                   filename, input);
   MultiVarData *mvdi = mode_exec->get_multivar_data(dtype);
   delete mode_exec; mode_exec = nullptr;

   //
   // create simple merged objects
   //
   _simple_objects(ModeExecutive::MULTIVAR_SIMPLE_MERGE, dtype, rIndex, tIndex, j, n_files,
                   filename, input);
   mode_exec->add_multivar_merge_data(mvdi, dtype);
   delete mode_exec;  mode_exec = nullptr;
   return mvdi;
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_simple_objects(
                          ModeExecutive::Processing_t p,
                          ModeDataType dtype, int rIndex, 
                          int tIndex, int j, int n_files,
                          const string &filename,
                          const ModeInputData &input) const
{
   if (dtype == ModeDataType::MvMode_Fcst) {
      _init_exec(p, filename, "None");
      mode_exec->init_multivar_simple(rIndex, tIndex, j, dtype, config);
      mode_exec->setup_multivar_fcst_data(verification_grid, input);
   } else {
      _init_exec(p, "None", filename);
      mode_exec->init_multivar_simple(rIndex, tIndex, j, dtype, config);
      mode_exec->setup_multivar_obs_data(verification_grid, input);
   }
   
   _simple_mode_algorithm(p, rIndex, tIndex);
}   

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_simple_mode_algorithm(ModeExecutive::Processing_t p,
                                              int rIndex, int tIndex) const
{
   mode_exec->clear_internal_r_index();
   mode_exec->do_conv_thresh_multivar_simple(p, rIndex, tIndex);
   mode_exec->clear_internal_r_index();
#ifdef  WITH_PYTHON
    GP.finalize();
 #endif
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_create_intensity_comparisons(
                          SimpleObjects &fcsts, int findex,
                          SimpleObjects &obs, int oindex,
                          const string &fcst_filename,
                          const string &obs_filename)
{
   MultiVarData *mvdf = fcsts._mvd[findex];
   MultiVarData *mvdo = obs._mvd[oindex];
   
   // mask the input data to be valid only inside the simple super objects
   fcsts._super.mask_data_simple("Fcst", *mvdf);
   obs._super.mask_data_simple("Obs", *mvdo);

   // this debug statement assumes fcsts and obs have same conv radius and thresh indices
   // which is currently required
   mlog << Debug(1) << "\n" << sep
        << "\nRunning mvmode intensity comparisions conv_radius[" << fcsts._rIndex+1
        << "] conv_thresh[" << fcsts._tIndex+1 << "]\n" << sep << "\n";

   _init_exec(ModeExecutive::MULTIVAR_INTENSITY, fcst_filename, obs_filename);
   mode_exec->init_multivar_intensities(config);

   ModeConfInfo & conf = mode_exec->engine.conf_info;
   conf.set_field_index(findex, oindex);

   // for multivar intensities, explicity set the level and units using stored values
   // from pass1
   conf.Fcst->var_info->set_level_name(mvdf->_level.c_str());
   conf.Fcst->var_info->set_units(mvdf->_units.c_str());
   if (fcsts._super._hasUnion && conf.Fcst->merge_flag == MergeType::Thresh) {
      mlog << Warning << "\nModeFrontEnd::_create_intensity_comparisons() -> "
           << "Logic includes union '||' along with  'merge_flag=THRESH' "
           << ". This can lead to bad results\n\n";
   }
   conf.Obs->var_info->set_level_name(mvdo->_level.c_str());
   conf.Obs->var_info->set_units(mvdo->_units.c_str());
   if (obs._super._hasUnion && conf.Obs->merge_flag == MergeType::Thresh) {
      mlog << Warning << "\nModeFrontEnd::_create_intensity_comparisons() -> "
           << "Logic includes union '||' along with  'merge_flag=THRESH' "
           << ". This can lead to bad results\n\n";
   }
       
   //
   // set up data access using inputs
   //
   mode_exec->setup_multivar_fcst_obs_data_intensities(*mvdf, *mvdo);

   //
   // run the mode algorithm for multivar intensities
   //
   _intensity_compare_mode_algorithm(fcsts._rIndex, fcsts._tIndex, obs._rIndex, obs._tIndex, *mvdf, *mvdo,
                                     fcsts._super, obs._super);

   delete mode_exec;  mode_exec = nullptr;
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_intensity_compare_mode_algorithm(
                          int rIndexF, int tIndexF, 
                          int rIndexO, int tIndexO,
                          const MultiVarData &mvdf,
                          const MultiVarData &mvdo,
                          const ModeSuperObject &fsuper,
                          const ModeSuperObject &osuper)
{
   mode_exec->do_conv_thresh_multivar_intensity_compare(rIndexF, tIndexF, rIndexO, tIndexO);
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

void MultivarFrontEnd::_process_superobjects(SimpleObjects &fcsts, SimpleObjects &obs)
{
   mlog << Debug(1) << "\n" << sep
        << "\nRunning mvmode superobject analysis conv_radius[" << fcsts._rIndex+1
        << "] conv_thresh[" << fcsts._tIndex+1 << "]\n" << sep << "\n";

   const MultiVarData *mvdf = fcsts._mvd[0];
   const MultiVarData *mvdo = obs._mvd[0];
   
   // set the data to 0 inside superobjects and missing everywhere else

   fcsts._super.mask_data_super("FcstSimple", *mvdf);
   obs._super.mask_data_super("ObsSimple", *mvdo);

   _init_exec(ModeExecutive::MULTIVAR_SUPER, "None", "None");
   mode_exec->init_multivar_intensities(config);

   const ModeConfInfo & conf = mode_exec->engine.conf_info;
   if ((fcsts._super._hasUnion || obs._super._hasUnion) &&
       (conf.Fcst->merge_flag == MergeType::Thresh ||
        conf.Obs->merge_flag == MergeType::Thresh)) {
      mlog << Warning << "\nModeFrontEnd::_process_superobjects() -> "
           << "Logic includes union '||' along with  'merge_flag=THRESH' "
           << ". This can lead to bad results\n\n";
   }
       
   //
   // set up data access using inputs
   //
   mode_exec->setup_multivar_fcst_obs_data_super(fcsts._super._simple_sd, obs._super._simple_sd,
                                                 *(mvdf->_grid));

   // run the mode algorithm
   _superobject_mode_algorithm(fcsts._rIndex, fcsts._tIndex, obs._rIndex, obs._tIndex,
                               fcsts._super, obs._super);

   delete mode_exec;  mode_exec = nullptr;
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_superobject_mode_algorithm(
                          int rIndexF, int tIndexF,
                          int rIndexO, int tIndexO,
                          const ModeSuperObject &fsuper,
                          const ModeSuperObject &osuper)

{
   mode_exec->clear_internal_r_index();
   mode_exec->do_conv_thresh_multivar_super(rIndexF, tIndexF, rIndexO, tIndexO);
   mode_exec->do_match_merge_multivar(fsuper._merge_sd_split, osuper._merge_sd_split,
                                      ModeExecutive::MULTIVAR_SUPER);
   mode_exec->process_output_multivar_super();
   mode_exec->clear_internal_r_index();
#ifdef  WITH_PYTHON
    GP.finalize();
 #endif
}

////////////////////////////////////////////////////////////////////////

void MultivarFrontEnd::_init_exec(
                          ModeExecutive::Processing_t p,
                          const string &ffile,
                          const string &ofile) const
{
   mlog << Debug(4) << "Running multivar front end for " << ModeExecutive::stype(p) << "\n";

   if ( mode_exec )  { delete mode_exec;  mode_exec = nullptr; }

   mode_exec = new ModeExecutive();
   mode_exec->fcst_file = ffile;
   mode_exec->obs_file = ofile;

   mode_exec->match_config_file = config_file; // this is never used
   mode_exec->out_dir = output_path;
}

////////////////////////////////////////////////////////////////////////
