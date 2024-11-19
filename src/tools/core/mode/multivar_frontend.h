// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MULTIVAR_FRONTEND_H__
#define  __MULTIVAR_FRONTEND_H__


////////////////////////////////////////////////////////////////////////

#include <string>
#include "mode_conf_info.h"
#include "two_d_array.h"
#include "bool_calc.h"
#include "multivar_data.h"
#include "mode_superobject.h"
#include "mode_input_data.h"
#include "mode_exec.h"

class MultivarFrontEnd {

private:

   int n_fcst_files, n_obs_files;
   StringArray fcst_filenames;
   StringArray  obs_filenames;
   BoolCalc f_calc, o_calc ;
   std::vector<ModeInputData> fcstInput, obsInput;
   std::vector<MultiVarData *> mvdFcst, mvdObs;
   std::string fcst_fof;
   std::string obs_fof;

   void _process_command_line(const StringArray &);
   void _read_config(const std::string & filename);
   void _setup_inputs();
   void _set_output_path();
   int  _mkdir(const char *dir);
   void _simple_objects(ModeExecutive::Processing_t p, ModeDataType dtype,
                        int j, int n_files, const std::string &filename,
                        const ModeInputData &input);
   void _init_exec(ModeExecutive::Processing_t p, const std::string &ffile, const std::string &ofile);
   void _superobject_mode_algorithm(const ModeSuperObject &fsuper, const ModeSuperObject &osuper);
   void _intensity_compare_mode_algorithm(const MultiVarData &mvdf, const MultiVarData &mvdo,
                                          const ModeSuperObject &fsuper, const ModeSuperObject &osuper);
   void _simple_mode_algorithm(ModeExecutive::Processing_t p);
   void _mode_algorithm_init() const;

public:

   bool do_clusters;
   std::string default_out_dir;
   ModeConfInfo config;
   ConcatString output_path;
   std::string   mode_path;
   std::string config_file;
   Grid verification_grid;

   MultivarFrontEnd();

   ~MultivarFrontEnd();


   int run(const StringArray & Argv);
   void init(const StringArray & Argv);

   static void set_outdir    (const StringArray &);
   static void set_logfile   (const StringArray &);
   static void set_verbosity (const StringArray &);
   static void set_compress  (const StringArray &);

   void read_input(const std::string &name, int index, ModeDataType type,
                   GrdFileType f_t, GrdFileType other_t, int shift);


   void create_verif_grid(void);

   MultiVarData *create_simple_objects(ModeDataType dtype, int j, int n_files,
                                       const std::string &filename,
                                       const ModeInputData &input);

   void create_intensity_comparisons(int findex, int oindex,
                                     const ModeSuperObject &fsuper,
                                     const ModeSuperObject &osuper,
                                     MultiVarData &mvdf, MultiVarData &mvdo,
                                     const std::string &fcst_filename,
                                     const std::string &obs_filename);

   void process_superobjects(ModeSuperObject &fsuper,
                             ModeSuperObject &osuper,
                             const MultiVarData &mvdf,
                             const MultiVarData &mvdo);

};


#endif   /*  __MULTIVAR_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////
