// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

#ifndef  __PAIR_DATA_ENSEMBLE_H__
#define  __PAIR_DATA_ENSEMBLE_H__

////////////////////////////////////////////////////////////////////////

#include <string>
#include <deque>
#include <map>

#include "pair_base.h"
#include "obs_error.h"

#include "vx_util.h"
#include "vx_grid.h"
#include "vx_data2d.h"
#include "vx_data2d_grib.h"
#include "vx_gsl_prob.h"


////////////////////////////////////////////////////////////////////////

class SSVARInfo; // forward reference

////////////////////////////////////////////////////////////////////////

// Structures to store the spread/skill point information
struct ens_ssvar_pt {
   double var;
   double f;
   double o;
   double w;
};

typedef std::deque<ens_ssvar_pt>             ssvar_pt_list;
typedef std::map<std::string,ssvar_pt_list>  ssvar_bin_map;  // Indexed by bin min
typedef CRC_Array<bool>                      BoolArray;
typedef CRC_Array<ObsErrorEntry *>           ObsErrorEntryPtrArray;

// Number of SSVAR bins to produce a warning
static const int n_warn_ssvar_bins = 1000;

////////////////////////////////////////////////////////////////////////
//
// Class to store ensemble pair data
//
////////////////////////////////////////////////////////////////////////

class PairDataEnsemble : public PairBase {

   private:

      void init_from_scratch();
      void assign(const PairDataEnsemble &);

   public:

      PairDataEnsemble();
      ~PairDataEnsemble();
      PairDataEnsemble(const PairDataEnsemble &);
      PairDataEnsemble & operator=(const PairDataEnsemble &);

      //////////////////////////////////////////////////////////////////

      // ObsErrorEntry points [n_obs]
      ObsErrorEntryPtrArray obs_error_entry;
      bool                  obs_error_flag;

      // Ensemble, valid count, and rank values
      NumArray  *e_na;            // Ensemble values [n_ens][n_obs]
      NumArray   v_na;            // Number of valid ensemble values [n_obs]
      NumArray   r_na;            // Observation ranks [n_obs]

      NumArray   crps_emp_na;       // Empirical Continuous Ranked Probability Score [n_obs]
      NumArray   crps_emp_fair_na;  // Fair Empirical Continuous Ranked Probability Score [n_obs]
      NumArray   spread_md_na;      // Mean absolute difference of ensemble members [n_obs]
      NumArray   crpscl_emp_na;     // Empirical climatological CRPS [n_obs]

      NumArray   crps_gaus_na;    // Gaussian CRPS [n_obs]
      NumArray   crpscl_gaus_na;  // Gaussian climatological CRPS [n_obs]

      NumArray   ign_na;          // Ignorance Score [n_obs]
      NumArray   pit_na;          // Probability Integral Transform [n_obs]

      NumArray   n_ge_obs_na;     // Number of ensemble memebers >= obs [n_obs]
      NumArray   me_ge_obs_na;    // Mean error of ensemble members >= obs [n_obs]
      NumArray   n_lt_obs_na;     // Number of ensemble members < obs [n_obs]
      NumArray   me_lt_obs_na;    // Mean error of ensemble members < obs [n_obs]

      int        n_ens;           // Number of ensemble members
      int        n_pair;          // Number of valid pairs, n_obs - sum(skip_ba)
      int        ctrl_index;      // Index of the control member
      bool       skip_const;      // Skip cases where the observation and
                                  // all ensemble members are constant
      BoolArray  skip_ba;         // Flag for each observation [n_obs]

      NumArray   rhist_na;        // Ranked Histogram [n_ens+1]
      NumArray   relp_na;         // Relative Position Histogram [n_ens]

      double     phist_bin_size;  // Ensemble PIT histogram bin width
      NumArray   phist_na;        // PIT Histogram [n_phist_bin]

      NumArray   var_na;           // Variance of unperturbed members [n_obs]
      NumArray   var_oerr_na;      // Variance of perturbed members [n_obs]
      NumArray   var_plus_oerr_na; // Unperturbed variance plus observation error variance [n_obs]

      NumArray   esum_na;         // Sum of unperturbed ensemble values [n_obs]
      NumArray   esumsq_na;       // Sum of unperturbed ensemble squared values [n_obs]
      NumArray   esumn_na;        // Count of ensemble values [n_obs]

      NumArray   mn_na;           // Ensemble mean value [n_obs]
      NumArray   mn_oerr_na;      // Mean of perturbed members [n_obs]

      double     ssvar_bin_size;  // Variance bin size for spread/skill
      SSVARInfo *ssvar_bins;      // Ensemble spread/skill bin information [n_ssvar_bin]

      double     crpss_emp;       // Empirical CRPS skill score
      double     crpss_gaus;      // Guassian CRPS skill score

      double     me;              // ME for ensemble mean
      double     mae;             // MAE for ensemble mean
      double     rmse;            // RMSE for ensemble mean
      double     me_oerr;         // ME for mean of perturbed members
      double     mae_oerr;        // MAE for mean of perturbed members
      double     rmse_oerr;       // RMSE for mean of perturbed members

      double     bias_ratio;      // Bias ratio

      //////////////////////////////////////////////////////////////////

      void clear();

      void extend(int);

      bool has_obs_error() const;

      void add_ens(int, double);
      void add_ens_var_sums(int, double);
      void set_ens_size(int);

      void add_obs_error_entry(ObsErrorEntry *);

      void compute_pair_vals(const gsl_rng *);

      void compute_rhist();
      void compute_relp();
      void compute_phist();
      void compute_ssvar();

      PairDataEnsemble subset_pairs_obs_thresh(const SingleThresh &ot) const;
};

////////////////////////////////////////////////////////////////////////
//
// Class to store a variety of PairDataEnsemble objects for each
// verification task
//
////////////////////////////////////////////////////////////////////////

class VxPairDataEnsemble {

   private:

      void init_from_scratch();
      void assign(const VxPairDataEnsemble &);

   public:

      VxPairDataEnsemble();
      ~VxPairDataEnsemble();
      VxPairDataEnsemble(const VxPairDataEnsemble &);
      VxPairDataEnsemble & operator=(const VxPairDataEnsemble &);

      //////////////////////////////////////////////////////////////////
      //
      // Information about the fields to be compared
      //
      //////////////////////////////////////////////////////////////////

      EnsVarInfo *fcst_info;     // Forecast field, allocated by EnsVarInfo
      VarInfo *climo_info;       // Climatology field, allocated by VarInfoFactory
      VarInfo *obs_info;         // Observation field, allocated by VarInfoFactory

      ConcatString desc;         // User description from config file

      double interp_thresh;      // Threshold between 0 and 1 used when
                                 // interpolating the forecasts to the
                                 // observation location.

      StringArray  msg_typ_sfc;  // List of surface message types

      //////////////////////////////////////////////////////////////////
      //
      // Forecast and climotology fields falling between the requested
      // levels.  Store the fields in a data plane array.
      //
      //////////////////////////////////////////////////////////////////

      DataPlaneArray fcst_dpa;     // Forecast data plane array
      DataPlaneArray climo_mn_dpa; // Climatology mean data plane array
      DataPlaneArray climo_sd_dpa; // Climatology standard deviation data plane array

      //////////////////////////////////////////////////////////////////

      unixtime fcst_ut;          // Ensemble valid time
      unixtime beg_ut;           // Beginning of valid time window
      unixtime end_ut;           // End of valid time window

      //////////////////////////////////////////////////////////////////

      StringArray sid_inc_filt;  // Station ID inclusion list
      StringArray sid_exc_filt;  // Station ID exclusion list
      StringArray obs_qty_inc_filt;  // Observation quality include markers
      StringArray obs_qty_exc_filt;  // Observation quality exclude markers
      
      //////////////////////////////////////////////////////////////////

      ObsErrorInfo *obs_error_info; // Pointer for observation error
                                    // Not allocated

      //////////////////////////////////////////////////////////////////

      int      n_msg_typ;        // Number of verifying message types

      int      n_mask;           // Total number of masking regions
                                 // of masking DataPlane fields or SIDs

      int      n_interp;         // Number of interpolation techniques

      //////////////////////////////////////////////////////////////////

      PairDataEnsemble ***pd;    // 3-Dim Array of PairDataEnsemble objects
                                 // as [n_msg_typ][n_mask][n_interp]

      //////////////////////////////////////////////////////////////////

      void clear();

      void set_fcst_info(EnsVarInfo *);
      void set_climo_info(VarInfo *);
      void set_obs_info(VarInfo *);

      void set_desc(const char *);

      void set_interp_thresh(double);
      void set_msg_typ_sfc(const StringArray &);

      void set_fcst_dpa(const DataPlaneArray &);
      void set_climo_mn_dpa(const DataPlaneArray &);
      void set_climo_sd_dpa(const DataPlaneArray &);

      void set_fcst_ut(const unixtime);
      void set_beg_ut(const unixtime);
      void set_end_ut(const unixtime);

      void set_sid_inc_filt(const StringArray);
      void set_sid_exc_filt(const StringArray);
      void set_obs_qty_inc_filt(const StringArray);
      void set_obs_qty_exc_filt(const StringArray);
      
      // Call set_pd_size before set_msg_typ, set_mask_area, and set_interp
      void set_pd_size(int, int, int);

      void set_msg_typ(int, const char *);
      void set_msg_typ_vals(int, const StringArray &);
      void set_mask_area(int, const char *, MaskPlane *);
      void set_mask_sid(int, const char *, StringArray *);
      void set_mask_llpnt(int, const char *, MaskLatLon *);

      void set_interp(int i_interp, const char *interp_mthd_str, int width,
                      GridTemplateFactory::GridTemplates shape);
      void set_interp(int i_interp, InterpMthd mthd, int width,
                      GridTemplateFactory::GridTemplates shape);

      // Call set_ens_size before add_ens
      void set_ens_size(int n);

      void set_climo_cdf_info_ptr(const ClimoCDFInfo *);

      void set_ssvar_bin_size(double);
      void set_phist_bin_size(double);

      void add_point_obs(float *, int *, const char *, const char *,
                         unixtime, const char *, float *, Grid &,
                         const char * = 0, const DataPlane * = 0);

      void add_ens(int, bool mn, Grid &);

      int  get_n_pair() const;

      void set_duplicate_flag(DuplicateType duplicate_flag);

      void set_obs_summary(ObsSummary obs_summary);

      void set_obs_perc_value(int percentile);

      void print_obs_summary();

      void calc_obs_summary();

      void set_ctrl_index(int);

      void set_skip_const(bool);
};

////////////////////////////////////////////////////////////////////////
//
// Miscellanous functions
//
////////////////////////////////////////////////////////////////////////

extern double compute_crps_emp(double, const NumArray &);
extern double compute_crps_gaus(double, double, double);
extern double compute_ens_ign(double, double, double);
extern double compute_ens_pit(double, double, double);
extern void   compute_bias_ratio_terms(double, const NumArray &,
                                       int &, double &, int &, double &);
extern double compute_bias_ratio(double, double);

////////////////////////////////////////////////////////////////////////

#endif   // __PAIR_DATA_ENSEMBLE_H__

////////////////////////////////////////////////////////////////////////
