// ** Copyright UCAR (c) 1992 - 2026
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
#include "simple_objects.hh"
#include "mode_superobject.h"
#include "mode_input_data.h"
#include "mode_exec.h"

class MultivarFrontEnd {

private:

   int n_fcst_files, n_obs_files;
   StringArray fcst_filenames;
   StringArray  obs_filenames;
   BoolCalc f_calc;
   BoolCalc o_calc;
   std::vector<ModeInputData> fcstInput;
   std::vector<ModeInputData>  obsInput;
   std::vector<MultiVarData *> mvdFcst;
   std::vector<MultiVarData *> mvdObs;
   std::string fcst_fof;
   std::string obs_fof;

   void _init(const StringArray & Argv);
   void _process_command_line(const StringArray &);
   void _read_config(const std::string & filename);
   void _setup_inputs();
   void _set_output_path();
   int  _mkdir(const char *dir) const;
   void _read_input(const std::string &name, int index, ModeDataType type,
                    GrdFileType f_t, GrdFileType other_t, int shift);
   void _create_verif_grid(void);

   void _create_simple_objects(ModeDataType dtype, const std::string &name,
                               int rIndex, int tIndex, int n_files,
                               const StringArray &filenames, const std::vector<ModeInputData> &input,
                               BoolCalc &calc, SimpleObjects &O) const;
   MultiVarData *_create_simple_multivar_data(ModeDataType dtype, int rIndex, int tIndex,
                                              int j, int n_files, const std::string &filename,
                                              const ModeInputData &input) const;
   void _simple_objects(ModeExecutive::Processing_t p, ModeDataType dtype, int rIndex,
                        int tIndex, int j, int n_files, const std::string &filename,
                        const ModeInputData &input) const;
   void _simple_mode_algorithm(ModeExecutive::Processing_t p, int rIndex, int tIndex) const;

   void _create_intensity_comparisons(SimpleObjects &fcsts, int findex, SimpleObjects &obs, int oindex,
                                      const string &fcst_filename, const string &obs_filename);
   void _intensity_compare_mode_algorithm(int rIndexF, int tIndexF, int rIndexO, int tIndexO,
                                          const MultiVarData &mvdf, const MultiVarData &mvdo,
                                          const ModeSuperObject &fsuper, const ModeSuperObject &osuper);

   void _process_superobjects(SimpleObjects &fcsts, SimpleObjects &obs);
   void _superobject_mode_algorithm(int rIndexF, int tIndexF, int rIndexO, int tIndexO,
                                    const ModeSuperObject &fsuper, const ModeSuperObject &osuper);

   void _init_exec(ModeExecutive::Processing_t p, const std::string &ffile, const std::string &ofile) const;
   
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

   static void set_outdir    (const StringArray &);
   static void set_logfile   (const StringArray &);
   static void set_verbosity (const StringArray &);
   static void set_compress  (const StringArray &);

};


#endif   /*  __MULTIVAR_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////
