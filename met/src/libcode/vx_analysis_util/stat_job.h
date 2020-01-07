// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////

#ifndef  __STAT_JOB_H__
#define  __STAT_JOB_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>

#include "stat_line.h"
#include "mask_poly.h"

#include "vx_cal.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_time_series.h"

////////////////////////////////////////////////////////////////////////

//
// Defaults to be used if not specified by the user
//
static const double default_bin_size     = 0.05;
static const double default_eclv_points  = 0.05;
static const bool   default_do_derive    = false;
static const bool   default_column_union = false;

//
// Ramp job type defaults
//
static const TimeSeriesType default_ramp_type            = TimeSeriesType_DyDt;
static const char           default_ramp_line_type[]     = "MPR";
static const char           default_ramp_out_line_type[] = "CTC,CTS";
static const char           default_ramp_fcst_col[]      = "FCST";
static const char           default_ramp_obs_col[]       = "OBS";
static const int            default_ramp_time            = 3600; // 1 hour
static const bool           default_ramp_exact           = true;
static const int            default_ramp_window          = 0;

//
// Dump buffer size
//
static const int dump_stat_buffer_rows = 512;
static const int dump_stat_buffer_cols = 512;

////////////////////////////////////////////////////////////////////////

//
// Enumerate all the possible STAT Analysis Job Types
//
enum STATJobType {

   stat_job_filter    = 0, // Filter out the STAT data and write the
                           // lines to the filename specified.

   stat_job_summary   = 1, // Compute min, max, mean, stdev and
                           // percentiles for a column of data.

   stat_job_aggr      = 2, // Aggregate the input counts/scores and
                           // generate the same output line type
                           // containing the aggregated counts/scores.

   stat_job_aggr_stat = 3, // Aggregate the input counts/scores and
                           // generate the requested output line type.

   stat_job_go_index  = 4, // Compute the GO Index.

   stat_job_ss_index  = 5, // Compute the Skill Score Index.

   stat_job_ramp      = 6, // Time-series ramp evaluation.

   no_stat_job_type   = 7  // Default value
};

static const int n_statjobtypes = 8;

static const char * const statjobtype_str[n_statjobtypes] = {
   "filter",         "summary",  "aggregate",
   "aggregate_stat", "go_index", "ss_index",
   "ramp",           "NA"
};

extern const char *statjobtype_to_string(const STATJobType);
extern void        statjobtype_to_string(const STATJobType, char *);
extern STATJobType string_to_statjobtype(const char *);

////////////////////////////////////////////////////////////////////////

class STATAnalysisJob {

   private:

      void init_from_scratch();

      void assign(const STATAnalysisJob &);

      // Output file precision
      int precision;

   public:

      STATAnalysisJob();
     ~STATAnalysisJob();
      STATAnalysisJob(const STATAnalysisJob &);
      STATAnalysisJob & operator=(const STATAnalysisJob &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      //////////////////////////////////////////////////////////////////

      void set_precision(int);
      int  get_precision() const;

      int  is_keeper(const STATLine &) const;

      double get_column_double(const STATLine &, const ConcatString &) const;

      void parse_job_command(const char *);
      void add_column_thresh(const char *, const char *);

      int  set_job_type (const char *);
      void set_dump_row (const char *);
      void set_stat_file(const char *);

      void set_mask_grid(const char *);
      void set_mask_poly(const char *);
      void set_mask_sid (const char *);

      void set_boot_rng (const char *);
      void set_boot_seed(const char *);

      void set_perc_thresh(const NumArray &, const NumArray &, const NumArray &);

      void open_dump_row_file ();
      void close_dump_row_file();
      void dump_stat_line     (const STATLine &);

      void open_stat_file ();
      void setup_stat_file(int n_row, int n);
      void close_stat_file();

      ConcatString get_case_info(const STATLine &) const;

      ConcatString get_jobstring() const;

      //
      // Job Type
      //
      STATJobType job_type;

      //
      // Variables used to stratify the input STAT lines
      //
      StringArray model;
      StringArray desc;

      IntArray    fcst_lead; // stored in seconds
      IntArray    obs_lead;  // stored in seconds

      unixtime    fcst_valid_beg;
      unixtime    fcst_valid_end;
      IntArray    fcst_valid_hour; // stored in seconds

      unixtime    obs_valid_beg;
      unixtime    obs_valid_end;
      IntArray    obs_valid_hour; // stored in seconds

      unixtime    fcst_init_beg;
      unixtime    fcst_init_end;
      IntArray    fcst_init_hour; // stored in seconds

      unixtime    obs_init_beg;
      unixtime    obs_init_end;
      IntArray    obs_init_hour;  // stored in seconds

      StringArray fcst_var;
      StringArray obs_var;

      StringArray fcst_units;
      StringArray obs_units;

      StringArray fcst_lev;
      StringArray obs_lev;

      StringArray obtype;

      StringArray vx_mask;

      StringArray interp_mthd;
      NumArray    interp_pnts;

      ThreshArray fcst_thresh;
      ThreshArray obs_thresh;
      ThreshArray cov_thresh;

      SetLogic    thresh_logic;

      NumArray    alpha;

      StringArray line_type;
      StringArray column;
      bool        column_union;
      NumArray    weight;

      // Numeric column thresholds
      map<ConcatString,ThreshArray> column_thresh_map;

      // ASCII column string matching
      map<ConcatString,StringArray> column_str_map;

      StringArray hdr_name;
      StringArray hdr_value;

      //
      // Store the case information for the -by option
      //
      StringArray by_column;

      //
      // Variables used to the store the analysis job specification
      //
      char        *dump_row; // dump rows used to a file
      ofstream    *dr_out;   // output file stream for dump row
      int         n_dump;    // number of lines written to dump row
      AsciiTable  dump_at;   // AsciiTable for buffering dump row data

      char        *stat_file; // dump output statistics to a STAT file
      ofstream    *stat_out;  // output file stream for -out_stat
      AsciiTable  stat_at;    // AsciiTable for buffering output STAT data

      StringArray  out_line_type;        // output line types
      ThreshArray  out_fcst_thresh;      // output forecast threshold(s)
      ThreshArray  out_obs_thresh;       // output observation threshold
      SetLogic     out_cnt_logic;        // output continuous thresholding logic
      SingleThresh out_fcst_wind_thresh; // output forecast wind speed threshold
      SingleThresh out_obs_wind_thresh;  // output observation wind speed threshold
      SetLogic     out_wind_logic;       // output wind speed thresholding logic
      double       out_alpha;            // output alpha value
      double       out_bin_size;         // output PHIST bin size
      NumArray     out_eclv_points;      // output ECLV points

      //
      // Variables used for the stat_job_summary job type
      //
      bool         do_derive;
      StringArray  wmo_sqrt_stats;
      StringArray  wmo_fisher_stats;

      //
      // Variables used for the stat_job_aggr_mpr job type
      //
      ConcatString mask_grid_str;
      ConcatString mask_poly_str;
      ConcatString mask_sid_str;

      Grid         mask_grid;
      MaskPlane    mask_area;
      MaskPoly     mask_poly;
      StringArray  mask_sid;

      //
      // Variables used for the stat_job_ramp job type
      //
      TimeSeriesType ramp_type;
      int            ramp_time_fcst;   // stored in seconds
      int            ramp_time_obs;
      bool           ramp_exact_fcst;  // exact or maximum change
      bool           ramp_exact_obs;
      SingleThresh   ramp_thresh_fcst; // ramp event definition
      SingleThresh   ramp_thresh_obs;
      int            ramp_window_beg;  // ramp event matching time window
      int            ramp_window_end;
      double         swing_width;      // swinging door algorithm width

      //
      // Variables used for computing bootstrap confidence intervals
      //

      //
      // Type of bootstrap confidence interval method:
      //    0 = BCa, 1 = Percentile (Default = 1)
      //
      int boot_interval;

      //
      // When using the percentile method, this is the proportion
      // of n to be resampled. (Default = 1.0)
      //
      double boot_rep_prop;

      //
      // Number of bootstrap replicates to be done. (Default = 1000)
      //
      int n_boot_rep;

      //
      // Name and seed value for the bootstrap random number generator.
      // (Default = "mnt19937" and "")
      //
      char *boot_rng;
      char *boot_seed;

      //
      // Rank correlation flag
      //
      int rank_corr_flag;

      //
      // Variance Inflation Factor flag
      //
      int vif_flag;

      int is_in_mask_grid(double, double) const;
      int is_in_mask_poly(double, double) const;
      int is_in_mask_sid (const char *)  const;
};

////////////////////////////////////////////////////////////////////////

inline void STATAnalysisJob::set_precision (int p)  { precision = p; return; }
inline int  STATAnalysisJob::get_precision () const { return(precision);     }

////////////////////////////////////////////////////////////////////////

#endif   /*  __STAT_JOB_H__  */

////////////////////////////////////////////////////////////////////////
