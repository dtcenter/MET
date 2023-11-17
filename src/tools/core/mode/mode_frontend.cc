// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


///////////////////////////////////////////////////////////////////////


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

#include "mode_frontend.h"
#include "mode_usage.h"

#ifdef WITH_PYTHON
#include "global_python.h"
#endif

extern const char * const program_name;

static ModeExecutive *mode_exec = 0;
static ModeExecutive::Processing_t ptype = ModeExecutive::TRADITIONAL;
static int compress_level = -1;


///////////////////////////////////////////////////////////////////////


ModeFrontEnd::ModeFrontEnd() :
   default_out_dir(".")
{
   mode_exec = 0;
   compress_level = -1;
}  


///////////////////////////////////////////////////////////////////////


ModeFrontEnd::~ModeFrontEnd()
{
   if ( mode_exec ) {
      delete mode_exec;  mode_exec = 0;
   }
}


///////////////////////////////////////////////////////////////////////


Grid ModeFrontEnd::create_verification_grid(const StringArray & Argv)

{
   if ( mode_exec )  { delete mode_exec;  mode_exec = 0; }
   mode_exec = new ModeExecutive;
   compress_level = -1;

   //
   // Process the command line arguments
   //

   process_command_line(Argv, false);

   mode_exec->init_multivar_verif_grid();

   ModeConfInfo & conf = mode_exec->engine.conf_info;
   conf.set_field_index(0);
   if (compress_level >= 0) conf.nc_info.set_compress_level(compress_level);


   //
   // read in data (Note multiple reads of same data)
   //
   mode_exec->setup_verification_grid();
   Grid g = mode_exec->grid;
   delete mode_exec;  mode_exec = 0;
   return g;
}


///////////////////////////////////////////////////////////////////////

int ModeFrontEnd::create_multivar_simple_objects(const StringArray & Argv, ModeDataType dtype,
                                                 const Grid &verification_grid, int field_index, int n_files)

{
   init(ModeExecutive::MULTIVAR_SIMPLE);

   //
   // Process the command line arguments
   //

   process_command_line_for_simple_objects(Argv, dtype);

   mode_exec->init_multivar_simple(n_files, dtype);

   ModeConfInfo & conf = mode_exec->engine.conf_info;
   if ( field_index >= 0 )  conf.set_field_index(field_index);
   if (compress_level >= 0) conf.nc_info.set_compress_level(compress_level);

   // need to do this after setting field index above
   mode_exec->check_multivar_perc_thresh_settings();

   //
   // read in data (Note multiple reads of data)
   //

   if (dtype == ModeDataType_MvMode_Fcst) {
      mode_exec->setup_fcst_data(verification_grid);
   } else {
      mode_exec->setup_obs_data(verification_grid);
   }
   
   //
   // mode algorithm
   //
   if ( conf.quilt )  {

      do_quilt();

   } else {

      do_straight();

   }

   //
   //  done
   //

#ifdef  WITH_PYTHON
   GP.finalize();
#endif
   return (0);
}

///////////////////////////////////////////////////////////////////////

int ModeFrontEnd::create_multivar_merge_objects(const StringArray & Argv, ModeDataType dtype,
                                                const Grid &verification_grid, int field_index,
                                                int n_files)

{
   init(ModeExecutive::MULTIVAR_SIMPLE_MERGE);

   //
   // Process the command line arguments
   //

   process_command_line_for_simple_objects(Argv, dtype);

   mode_exec->init_multivar_simple(n_files, dtype);

   ModeConfInfo & conf = mode_exec->engine.conf_info;
   if ( field_index >= 0 )  conf.set_field_index(field_index);
   if (compress_level >= 0) conf.nc_info.set_compress_level(compress_level);

   // need to do this after setting field index above
   mode_exec->check_multivar_perc_thresh_settings();


   //
   // read in data (Note multiple reads not desired)
   //

   if (dtype == ModeDataType_MvMode_Fcst) {
      mode_exec->setup_fcst_data(verification_grid);
   } else {
      mode_exec->setup_obs_data(verification_grid);
   }
   

   //
   // mode algorithm
   //
   if ( conf.quilt )  {

      do_quilt();

   } else {

      do_straight();

   }

   //
   //  done
   //

#ifdef  WITH_PYTHON
   GP.finalize();
#endif
   return (0);
}

///////////////////////////////////////////////////////////////////////

int ModeFrontEnd::run_traditional(const StringArray & Argv) 

{
   init(ModeExecutive::TRADITIONAL);

   int field_index = -1;
   int n_files = 1;

   //
   // Process the command line arguments
   //

   process_command_line(Argv, false);

   mode_exec->init_traditional(n_files);

   ModeConfInfo & conf = mode_exec->engine.conf_info;
   if ( field_index >= 0 )  conf.set_field_index(field_index);
   if (compress_level >= 0) conf.nc_info.set_compress_level(compress_level);


   //
   // read in data
   //

   mode_exec->setup_fcst_obs_data_traditional();

   //
   // mode algorithm
   //
   if ( conf.quilt )  {

      do_quilt();

   } else {

      do_straight();

   }

   //
   //  done
   //

#ifdef  WITH_PYTHON
   GP.finalize();
#endif
   return (0);
}

///////////////////////////////////////////////////////////////////////

int ModeFrontEnd::multivar_intensity_comparisons(const StringArray & Argv, const MultiVarData &mvdf,
                                                 const MultiVarData &mvdo, bool has_union_f,
                                                 bool has_union_o, ShapeData &merge_f,
                                                 ShapeData &merge_o, int field_index_f, int field_index_o)
{
   init(ModeExecutive::MULTIVAR_INTENSITY);

   //
   // Process the command line arguments
   //

   process_command_line(Argv, false);

   mode_exec->init_multivar_intensities(mvdf._type, mvdo._type);

   ModeConfInfo & conf = mode_exec->engine.conf_info;
   if (compress_level >= 0) conf.nc_info.set_compress_level(compress_level);
   conf.set_field_index(field_index_f, field_index_o);

   // for multivar intensities, explicity set the level and units using stored values
   // from pass1
   conf.Fcst->var_info->set_level_name(mvdf._level.c_str());
   conf.Fcst->var_info->set_units(mvdf._units.c_str());
   if (has_union_f && conf.Fcst->merge_flag == MergeType_Thresh) {
      mlog << Warning << "\nModeFrontEnd::multivar_intensity_comparisons() -> "
           << "Logic includes union '||' along with  'merge_flag=THRESH' "
           << ". This can lead to bad results\n\n";
   }
   conf.Obs->var_info->set_level_name(mvdo._level.c_str());
   conf.Obs->var_info->set_units(mvdo._units.c_str());
   if (has_union_o && conf.Obs->merge_flag == MergeType_Thresh) {
      mlog << Warning << "\nModeFrontEnd::multivar_intensity_comparisons() -> "
           << "Logic includes union '||' along with  'merge_flag=THRESH' "
           << ". This can lead to bad results\n\n";
   }
       
   //
   // set up data access using inputs
   //
   mode_exec->setup_fcst_obs_data_multivar_intensities(mvdf, mvdo);

   //
   // run the mode algorithm
   //

   if ( conf.quilt )  {

      do_quilt();

   } else {

      do_straight_multivar_intensity(mvdf, mvdo, merge_f, merge_o);

   }

   //
   //  done
   //

#ifdef  WITH_PYTHON
   GP.finalize();
#endif
   return (0);
}

///////////////////////////////////////////////////////////////////////

int ModeFrontEnd::run_super(const StringArray & Argv, 
                            ShapeData &f_super, ShapeData &o_super,
                            ShapeData &f_merge, ShapeData &o_merge,
                            GrdFileType ftype, GrdFileType otype, const Grid &grid,
                            bool has_union)
{
   init(ModeExecutive::MULTIVAR_SUPER);

   //
   // Process the command line arguments
   //

   process_command_line(Argv, true);

   mode_exec->init_multivar_intensities(ftype, otype);

   ModeConfInfo & conf = mode_exec->engine.conf_info;
   if (compress_level >= 0) conf.nc_info.set_compress_level(compress_level);
   if (has_union && (conf.Fcst->merge_flag == MergeType_Thresh ||
                     conf.Obs->merge_flag == MergeType_Thresh)) {
      mlog << Warning << "\nModeFrontEnd::run_super() -> "
           << "Logic includes union '||' along with  'merge_flag=THRESH' "
           << ". This can lead to bad results\n\n";
   }
       
   //
   // set up data access using inputs
   //
   mode_exec->setup_fcst_obs_data_multivar_super(f_super, o_super, grid);

   //
   // run the mode algorithm
   //

   if ( conf.quilt )  {

      do_quilt();

   } else {

      do_straight_multivar_super(f_merge, o_merge);

   }

   //
   //  done
   //

#ifdef  WITH_PYTHON
   GP.finalize();
#endif
   return (0);
}

///////////////////////////////////////////////////////////////////////


void ModeFrontEnd::do_straight()

{
   int NCT, NCR;

   do_straight_init(NCT, NCR);

   mode_exec->clear_internal_r_index();

   for (int index=0; index<NCT; ++index)  {

      mode_exec->do_conv_thresh(index, index);
      if (ptype == ModeExecutive::TRADITIONAL) {

         mode_exec->do_match_merge();
         mode_exec->process_output();
      }
   }

   mode_exec->clear_internal_r_index();
  
   //
   //  done
   //

   return;

}

///////////////////////////////////////////////////////////////////////


void ModeFrontEnd::do_straight_multivar_intensity(const MultiVarData &mvdf,
                                                  const MultiVarData &mvdo,
                                                  ShapeData &f_merge,
                                                  ShapeData &o_merge)

{
   int NCT, NCR;
   
   do_straight_init(NCT, NCR);

   mode_exec->clear_internal_r_index();

   for (int index=0; index<NCT; ++index)  {

      mode_exec->do_conv_thresh(index, index);
      mode_exec->do_match_merge(f_merge, o_merge);

      // here replace raw data and min/max for plotting

      mode_exec->process_output(&mvdf, &mvdo);
   }

   mode_exec->clear_internal_r_index();
  
   //
   //  done
   //

   return;

}



///////////////////////////////////////////////////////////////////////


void ModeFrontEnd::do_straight_multivar_super(ShapeData &f_merge, ShapeData &o_merge)

{
   int NCT, NCR;
   
   do_straight_init(NCT, NCR);

   mode_exec->clear_internal_r_index();

   for (int index=0; index<NCT; ++index)  {

      mode_exec->do_conv_thresh(index, index);
      mode_exec->do_match_merge(f_merge, o_merge);
      mode_exec->process_output();
   }

   mode_exec->clear_internal_r_index();
  
   //
   //  done
   //

   return;

}


///////////////////////////////////////////////////////////////////////


void ModeFrontEnd::do_quilt()

{
   if (ptype != ModeExecutive::TRADITIONAL) {
      mlog << Error << "\nModeFrontend::do_quilt() -> quilting not yet implemented for multivar mode \n\n";
      exit ( 1 );
   }
      

   int t_index, r_index;   //  indices into the convolution threshold and radius arrays


   mode_exec->clear_internal_r_index();

   for (r_index=0; r_index<(mode_exec->n_conv_radii()); ++r_index)  {

      for (t_index=0; t_index<(mode_exec->n_conv_threshs()); ++t_index)  {

         mode_exec->do_conv_thresh(r_index, t_index);

         mode_exec->do_match_merge();

         if (ptype == ModeExecutive::TRADITIONAL) {
            mode_exec->process_output();
         }
      }
   }

   mode_exec->clear_internal_r_index();

   //
   //  done
   //

   return;

}

///////////////////////////////////////////////////////////////////////

MultiVarData *ModeFrontEnd::get_multivar_data(ModeDataType dtype)
{
   return mode_exec->get_multivar_data(dtype);
}


///////////////////////////////////////////////////////////////////////

void ModeFrontEnd::add_multivar_merge_data(MultiVarData *mvdi, ModeDataType dtype)
{
   return mode_exec->add_multivar_merge_data(mvdi, dtype);
}

///////////////////////////////////////////////////////////////////////

void ModeFrontEnd::init(ModeExecutive::Processing_t p)
{
   ptype = p;
   mlog << Debug(1) << "Running multivar front end for " << ModeExecutive::stype(ptype) << "\n";

   if ( mode_exec )  { delete mode_exec;  mode_exec = 0; }

   mode_exec = new ModeExecutive(ptype);
   compress_level = -1;
}

///////////////////////////////////////////////////////////////////////

void ModeFrontEnd::do_straight_init(int &NCT, int &NCR) const
{
   const ModeConfInfo & conf = mode_exec->engine.conf_info;

   NCT = conf.n_conv_threshs();
   NCR = conf.n_conv_radii();

   if ( NCT != NCR )  {

      mlog << Error << "\nModeFrontEnd::do_straight_init() ->"
           << "all convolution radius and threshold arrays must have the same number of elements\n\n";

      exit ( 1 );

   }

   if (NCT > 1 && ptype != ModeExecutive::TRADITIONAL) {

      mlog << Error << "\nModeFrontEnd::do_straight_init() ->"
           << ": multiple convolution radii and thresholds not implemented in multivar mode\n\n";

      exit ( 1 );
   }
}

      
///////////////////////////////////////////////////////////////////////

void ModeFrontEnd::process_command_line(const StringArray & argv, bool ismultivar)
{
   CommandLine cline;
   ConcatString s;
   const int argc = argv.n();

   //
   // Set the default output directory
   //

   mode_exec->out_dir = replace_path(default_out_dir);

   //
   // Check for zero arguments (note not correct for multivar mode, want to show multivar_usage
   //

   if(argc == 1) singlevar_usage();

   //
   // Parse the command line into tokens
   //

   cline.set(argv);

   //
   // Set the usage function NOTE wrong for multivar, want multivar_usage
   //

   cline.set_usage(singlevar_usage);

   //
   // Add the options function calls
   //

   cline.add(set_config_merge_file, "-config_merge", 1);
   cline.add(set_outdir,            "-outdir",       1);
   cline.add(set_logfile,           "-log",          1);
   cline.add(set_verbosity,         "-v",            1);
   cline.add(set_compress,          "-compress",     1);

   //
   // Parse the command line
   //

   cline.parse();

   if (ismultivar) {
      //
      // Check for error. There should be 1 argument left:
      // config filename
      //
      if(cline.n() != 1) singlevar_usage();  // wrong need multivar usage

      //
      // Store the input forecast and observation file names, placeholders
      //
      mode_exec->fcst_file         = "not set";
      mode_exec->obs_file          = "not set";
      mode_exec->match_config_file = cline[0];

   } else {
      //
      // Check for error. There should be three arguments left:
      // forecast, observation, and config filenames
      //
      if(cline.n() != 3) singlevar_usage();

      //
      // Store the input forecast and observation file names
      //
      mode_exec->fcst_file         = cline[0];
      mode_exec->obs_file          = cline[1];
      mode_exec->match_config_file = cline[2];

   }
}

      
///////////////////////////////////////////////////////////////////////

void ModeFrontEnd::process_command_line_for_simple_objects(const StringArray &argv, ModeDataType dtype)
{
   CommandLine cline;
   ConcatString s;
   const int argc = argv.n();

   //
   // Set the default output directory
   //

   mode_exec->out_dir = replace_path(default_out_dir);

   //
   // Check for zero arguments (note not correct for multivar mode, want to show multivar_usage
   //

   if(argc == 1) singlevar_usage();

   //
   // Parse the command line into tokens
   //

   cline.set(argv);

   //
   // Set the usage function NOTE wrong for multivar, want multivar_usage
   //

   cline.set_usage(singlevar_usage);

   //
   // Add the options function calls
   //

   cline.add(set_config_merge_file, "-config_merge", 1);
   cline.add(set_outdir,            "-outdir",       1);
   cline.add(set_logfile,           "-log",          1);
   cline.add(set_verbosity,         "-v",            1);
   cline.add(set_compress,          "-compress",     1);

   //
   // Parse the command line
   //

   cline.parse();

   //
   // Check for error. There should be two arguments left:
   // data and config filenames
   //
   if(cline.n() != 2) singlevar_usage();

   //
   // Store the file name
   //
   if (dtype == ModeDataType_MvMode_Fcst) {
      mode_exec->fcst_file         = cline[0];
      mode_exec->obs_file          = "None";
   } else {
      mode_exec->obs_file         = cline[0];
      mode_exec->fcst_file          = "None";
   }
   mode_exec->match_config_file = cline[1];
}

///////////////////////////////////////////////////////////////////////

void ModeFrontEnd::set_config_merge_file(const StringArray & a)
{
   mode_exec->merge_config_file = a[0];
}

////////////////////////////////////////////////////////////////////////

void ModeFrontEnd::set_outdir(const StringArray & a)
{
   mode_exec->out_dir = a[0];
}

////////////////////////////////////////////////////////////////////////

void ModeFrontEnd::set_logfile(const StringArray & a)
{
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void ModeFrontEnd::set_verbosity(const StringArray & a)
{
   mlog.set_verbosity_level(atoi(a[0].c_str()));
}

////////////////////////////////////////////////////////////////////////

void ModeFrontEnd::set_compress(const StringArray & a)
{
   compress_level = atoi(a[0].c_str());
}

////////////////////////////////////////////////////////////////////////



