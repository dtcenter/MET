// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_EXECUTIVE_H__
#define  __MODE_EXECUTIVE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include <netcdf>

#include "vx_data2d.h"
#include "vx_data2d_factory.h"
#include "data2d_factory_utils.h"
#include "vx_shapedata.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_nc_util.h"
#include "vx_cal.h"
#include "vx_grid.h"
#include "vx_math.h"
#include "vx_statistics.h"

#include "vx_color.h"
#include "vx_ps.h"
#include "vx_pxm.h"
#include "vx_render.h"
#include "vx_plot_util.h"

#include "mode_ps_file.h"
#include "mode_data_type.h"
#include "multivar_data.h"

////////////////////////////////////////////////////////////////////////


static const int n_cts = 2;


////////////////////////////////////////////////////////////////////////


enum class ObjPolyType {
   FcstSimpBdy  = 0,
   ObsSimpBdy   = 1,
   FcstSimpHull = 2,
   ObsSimpHull  = 3,
   FcstClusHull = 4,
   ObsClusHull  = 5
};


////////////////////////////////////////////////////////////////////////


class ModeExecutive {

 private:

   void init_from_scratch();

 public:

   // the various mode algorithm settings
   typedef enum {TRADITIONAL, MULTIVAR_SIMPLE, MULTIVAR_SIMPLE_MERGE, MULTIVAR_INTENSITY, MULTIVAR_SUPER} Processing_t;

   ModeExecutive();
   ~ModeExecutive();

   void clear();

   void init_traditional(int n_files);
   void init_multivar_simple(int j, int n_files, ModeDataType dtype, const ModeConfInfo &conf);
   void init_multivar_intensities(const ModeConfInfo &conf);

   int n_conv_radii   () const;
   int n_conv_threshs () const;
   int n_runs() const;

   // these are used only for traditional mode
   int R_index;   //  indices into the convolution radius and threshold arrays
   int T_index;   //  for the current run

   //
   // Input configuration files, all 3 used only for traditional mode
   // Multivar mode handles the configs outside of the exec
   //
   // the hardwired default traditional mode config
   ConcatString default_config_file;

   // set for both trad and multivar, this is the default file on the command line
   // used only for traditional
   ConcatString match_config_file;    

   // the extra one that can be set only for traditional mode
   ConcatString merge_config_file;

   //
   // Input filenames, set for both multivar and trad mode
   // but used only for trad mode
   //
   ConcatString fcst_file;
   ConcatString obs_file;

   // set and used only for trad mode
   Met2dDataFile * fcst_mtddf;
   Met2dDataFile * obs_mtddf;

   // used for both trad and multivar mode
   TTContingencyTable cts[n_cts];

   // used for both trad and multivar mode
   ModeFuzzyEngine engine;

   // verification grid
   // used for both trad and multivar mode, set by both
   Grid grid;

   Box xy_bb;
   ConcatString out_dir;

   // set for both trad and multivar mode, used for plotting limits
   double data_min, data_max;

   // set for trad and multivar, used in the engine mode algorithm
   ShapeData Fcst_sd, Obs_sd;

   // not used by multivar
   GrdFileType ftype, otype;

   // set into execs's conf varInfo object, only for multivar intensity comparisons
   // for trad it's read in from the config
   string funits, ounits;

   // set into execs's conf varInfo object, only for multivar intensity comparisons
   // for trad it's read in from the config
   string flevel, olevel;

   // used in multivar only to customize outputs correctly
   bool isMultivarOutput;
   bool isMultivarSuperOutput;
   
   void setup_verification_grid(const ModeInputData &fcst,
                                const ModeInputData &obs,
                                const ModeConfInfo &conf);

   void clear_internal_r_index();

   void setup_traditional_fcst_obs_data();
   void setup_multivar_fcst_data(const Grid &verification_grid, const ModeInputData &input);
   void setup_multivar_obs_data(const Grid &verification_grid, const ModeInputData &input);
   void setup_multivar_fcst_obs_data_intensities(const MultiVarData &mvdf,
                                                 const MultiVarData &mvdo);
   void setup_multivar_fcst_obs_data_super(const ShapeData &f_super,
                                           const ShapeData &o_super,
                                           const Grid &igrid);

   void do_conv_thresh_traditional(const int r_index, const int t_index);
   void do_conv_thresh_multivar_super();
   void do_conv_thresh_multivar_intensity_compare();
   void do_conv_thresh_multivar_simple(Processing_t p);

   void do_merging_traditional();
   void do_merging_multivar(const ShapeData &f_merge, const ShapeData &o_merge,
                            Processing_t p);

   void do_match_merge_traditional();
   void do_match_merge_multivar(const ShapeData &f_merge, const ShapeData &o_merge,
                                Processing_t p);

   void process_masks(ShapeData &, ShapeData &);
   void process_fcst_masks(ShapeData &);
   void process_obs_masks(ShapeData &);

   void process_output_traditional();
   void process_output_multivar_intensity_compare(const MultiVarData *mvdf,
                                                  const MultiVarData *mvdo);
   void process_output_multivar_super();

   void set_raw_to_full(float *fcst_raw_data,
                        float *obs_raw_data,
                        int nx, int ny,
                        double data_min, double data_max);
      
  
   // owned by caller
   MultiVarData *get_multivar_data(ModeDataType dtype);
   void add_multivar_merge_data(MultiVarData *mvdi, ModeDataType dtype);

   void plot_engine();

   void compute_ct_stats();

   void build_outfile_prefix (ConcatString &);

   void build_simple_outfile_name  (const char *, ConcatString &);
   void build_outfile_name         (const char *, ConcatString &);

   void write_obj_stats();
   void write_obj_netcdf(const ModeNcOutInfo &);
   void write_poly_netcdf(netCDF::NcFile *);
   void write_poly_netcdf(netCDF::NcFile *, const ObjPolyType);
   void write_ct_stats();

   // traditional only, multivar reads outside of the exec
   void conf_read();

   static string stype(Processing_t t);

};


////////////////////////////////////////////////////////////////////////


inline int ModeExecutive::n_conv_radii   () const { return ( engine.conf_info.n_conv_radii   () ); }
inline int ModeExecutive::n_conv_threshs () const { return ( engine.conf_info.n_conv_threshs () ); }

inline int ModeExecutive::n_runs () const { return ( engine.conf_info.n_runs () ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_EXECUTIVE_H__  */


/////////////////////////////////////////////////////////////////////////
