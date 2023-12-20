// ** Copyright UCAR (c) 1992 - 2023
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


enum ObjPolyType {
   FcstSimpBdyPoly  = 0,
   ObsSimpBdyPoly   = 1,
   FcstSimpHullPoly = 2,
   ObsSimpHullPoly  = 3,
   FcstClusHullPoly = 4,
   ObsClusHullPoly  = 5
};


////////////////////////////////////////////////////////////////////////


class ModeExecutive {

 private:

   void init_from_scratch();

 public:

   // the various mode algorithm settings
   typedef enum {TRADITIONAL, MULTIVAR_SIMPLE, MULTIVAR_SIMPLE_MERGE, MULTIVAR_INTENSITY, MULTIVAR_SUPER} Processing_t;

   ModeExecutive(Processing_t p=TRADITIONAL);
   ~ModeExecutive();

   void clear();

   void init_traditional(int n_files);
   void init_multivar_verif_grid(const DataPlane &fcst,
                                 const DataPlane &obs, const ModeConfInfo &config);
   void init_multivar_simple(int j, int n_files, ModeDataType dtype, const ModeConfInfo &conf);
   void init_multivar_intensities(GrdFileType ftype, GrdFileType otype, const ModeConfInfo &conf);


   int n_conv_radii   () const;
   int n_conv_threshs () const;

   int n_runs() const;

   int R_index;   //  indices into the convolution radius and threshold arrays
   int T_index;   //  for the current run

   //
   // Input configuration files
   //

   ConcatString default_config_file;
   ConcatString match_config_file;
   ConcatString merge_config_file;

   //
   // Input files
   //

   ConcatString fcst_file;
   ConcatString obs_file;
   Met2dDataFile * fcst_mtddf;
   Met2dDataFile * obs_mtddf;

   TTContingencyTable cts[n_cts];

   ModeFuzzyEngine engine;

   // verification grid
   Grid grid;

   Box xy_bb;
   ConcatString out_dir;
   double data_min, data_max;

   ShapeData Fcst_sd, Obs_sd;

   GrdFileType ftype, otype;
   string funits, ounits;
   string flevel, olevel;

   bool isMultivarOutput;
   bool isMultivarSuperOutput;
   
   Processing_t ptype;
   
   void clear_internal_r_index();
   void setup_verification_grid(const ModeInputData &fcst,
                                const ModeInputData &obs);
   void setup_fcst_obs_data_traditional();
   void setup_fcst_data(const Grid &verification_grid, const ModeInputData &input);
   void setup_obs_data(const Grid &verification_grid, const ModeInputData &input);
   void setup_fcst_obs_data_multivar_intensities(const MultiVarData &mvdf,
                                                 const MultiVarData &mvdo);
   void setup_fcst_obs_data_multivar_super(const ShapeData &f_super,
                                           const ShapeData &o_super, const Grid &igrid);
   void do_conv_thresh(const int r_index, const int t_index);
   void do_merging();
   void do_merging(const ShapeData &f_merge, const ShapeData &o_merge);
   void do_match_merge();
   void do_match_merge(const ShapeData &f_merge, const ShapeData &o_merge);

   void process_masks(ShapeData &, ShapeData &);
   void process_fcst_masks(ShapeData &);
   void process_obs_masks(ShapeData &);
   void process_output(const MultiVarData *mvdf=NULL,
                       const MultiVarData *mvdo=NULL);

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

   void conf_read(const string &default_config_filename);
   static string stype(Processing_t t);

};


////////////////////////////////////////////////////////////////////////


inline int ModeExecutive::n_conv_radii   () const { return ( engine.conf_info.n_conv_radii   () ); }
inline int ModeExecutive::n_conv_threshs () const { return ( engine.conf_info.n_conv_threshs () ); }

inline int ModeExecutive::n_runs () const { return ( engine.conf_info.n_runs () ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_EXECUTIVE_H__  */


/////////////////////////////////////////////////////////////////////////
