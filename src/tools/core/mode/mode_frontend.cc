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
static int compress_level = -1;
static int field_index = -1;


///////////////////////////////////////////////////////////////////////


ModeFrontEnd::ModeFrontEnd() :
   default_out_dir(".")
{
   mode_exec = 0;
   compress_level = -1;
   field_index = -1;
}  


///////////////////////////////////////////////////////////////////////


ModeFrontEnd::~ModeFrontEnd()
{
   if ( mode_exec ) {
      delete mode_exec;  mode_exec = 0;
   }
}

///////////////////////////////////////////////////////////////////////

int ModeFrontEnd::run(const StringArray & Argv, Processing_t ptype)

{

   // Argv.dump(cout, 0);
   if ( mode_exec )  { delete mode_exec;  mode_exec = 0; }
   mode_exec = new ModeExecutive;
   compress_level = -1;
   field_index = -1;


   //
   // Process the command line arguments
   //

   process_command_line(Argv);

   ModeConfInfo & conf = mode_exec->engine.conf_info;
   if ( field_index >= 0 )  conf.set_field_index(field_index);
   if (compress_level >= 0) conf.nc_info.set_compress_level(compress_level);


   //
   // read in data
   //

   mode_exec->setup_fcst_obs_data();

   //
   // mode algorithm
   //
   if ( conf.quilt )  {

      do_quilt(ptype);

   } else {

      do_straight(ptype);

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

int ModeFrontEnd::run(const StringArray & Argv, const MultiVarData &mvd)
{
   if ( mode_exec )  { delete mode_exec;  mode_exec = 0; }

   mode_exec = new ModeExecutive;
   compress_level = -1;
   field_index = -1;

   //Argv.dump(cout, 0);

   //
   // Process the command line arguments
   //

   process_command_line_final(Argv, mvd);
   ModeConfInfo & conf = mode_exec->engine.conf_info;
   if (compress_level >= 0) conf.nc_info.set_compress_level(compress_level);
   //if ( field_index >= 0 )  conf.set_field_index(field_index);

   //
   // set up data access using inputs
   //
   mode_exec->setup_fcst_obs_data(mvd);

   //
   // run the mode algorithm
   //

   if ( conf.quilt )  {

      do_quilt(MULTIVAR_PASS2);

   } else {

      do_straight(MULTIVAR_PASS2);

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


void ModeFrontEnd::do_straight(Processing_t ptype)

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

   if (NCT > 1 && ptype != SINGLE_VAR) {
      mlog << Error
           << "\n\n  "
           << program_name
           << ": multiple convolution radii and thresholds not implemented in multivar mode\n\n";

         exit ( 1 );
   }
      
      

   int index;

   mode_exec->clear_internal_r_index();

   for (index=0; index<NCT; ++index)  {

      mode_exec->do_conv_thresh(index, index);

      mode_exec->do_match_merge();

      if (ptype == SINGLE_VAR || ptype == MULTIVAR_PASS2) {
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


void ModeFrontEnd::do_quilt(Processing_t ptype)

{

   int t_index, r_index;   //  indices into the convolution threshold and radius arrays


   mode_exec->clear_internal_r_index();

   for (r_index=0; r_index<(mode_exec->n_conv_radii()); ++r_index)  {

      for (t_index=0; t_index<(mode_exec->n_conv_threshs()); ++t_index)  {

         mode_exec->do_conv_thresh(r_index, t_index);

         mode_exec->do_match_merge();

         if (ptype == SINGLE_VAR || ptype == MULTIVAR_PASS2) {
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

MultiVarData *ModeFrontEnd::get_multivar_data() {return mode_exec->get_multivar_data(); }

///////////////////////////////////////////////////////////////////////

void ModeFrontEnd::process_command_line(const StringArray & argv)
{

   CommandLine cline;
   ConcatString s;
   const int argc = argv.n();

   //
   // Set the default output directory
   //

   mode_exec->out_dir = replace_path(default_out_dir);

   //
   // Check for zero arguments
   //

   if(argc == 1) singlevar_usage();

   //
   // Parse the command line into tokens
   //

   cline.set(argv);

   //
   // Set the usage function
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
   //  add for mode multivar ... undocumented
   //

   cline.add(set_field_index, "-field_index", 1);

   //
   // Parse the command line
   //

   cline.parse();

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

   mode_exec->init();

   return;

}

///////////////////////////////////////////////////////////////////////

void ModeFrontEnd::process_command_line_final(const StringArray & argv,
                                              const MultiVarData &mvd)
{

   CommandLine cline;
   ConcatString s;
   const int argc = argv.n();

   //
   // Set the default output directory
   //

   mode_exec->out_dir = replace_path(default_out_dir);

   //
   // Check for zero arguments
   //

   if(argc == 1) singlevar_usage();  // wrong, need something else

   //
   // Parse the command line into tokens
   //

   cline.set(argv);

   //
   // Set the usage function
   //

   cline.set_usage(singlevar_usage);  // wrong, need something else

   //
   // Add the options function calls
   //

   cline.add(set_config_merge_file, "-config_merge", 1);
   cline.add(set_outdir,            "-outdir",       1);
   cline.add(set_logfile,           "-log",          1);
   cline.add(set_verbosity,         "-v",            1);
   cline.add(set_compress,          "-compress",     1);

   //
   //  add for mode multivar ... undocumented
   //
   //cline.add(set_field_index, "-field_index", 1);

   //
   // Parse the command line
   //

   cline.parse();

   //
   // Check for error. There should be 1 argument left:
   // config filename
   //

   if(cline.n() != 1) singlevar_usage();  // wrong need something else

   //
   // Store the input forecast and observation file names, placeholders
   //

   mode_exec->fcst_file         = "not set";
   mode_exec->obs_file          = "not set";
   mode_exec->match_config_file = cline[0];
   mode_exec->init_final(mvd);

   return;

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

void ModeFrontEnd::set_field_index(const StringArray & a)

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



