// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_STAT_JOB_H__
#define  __TC_STAT_JOB_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>

#include "tc_stat_files.h"

#include "met_stats.h"
#include "mask_poly.h"
#include "vx_tc_util.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

// Defaults to be used if not specified by the user
static const bool         default_column_union = false;
static const bool         default_water_only   = false;
static const bool         default_match_points = false;
static const bool         default_event_equal  = false;

// Default rapid intensification is an increase of 30 kts over 24 hours
static const TrackType    default_rirw_track  = TrackType_None;
static const int          default_rirw_time   = 86400;
static const bool         default_rirw_exact  = true;
static const SingleThresh default_rirw_thresh(">=30.0");
static const int          default_rirw_window = 0;

// Defaults for the PROBRIRW job type
static const double       default_probrirw_thresh = bad_data_double;
static const bool         default_probrirw_exact  = false;
static const SingleThresh default_probrirw_bdelta_thresh(">=30");
static const char         default_probrirw_prob_thresh[] = "==0.1";

// Default is 24 hours prior to landfall
static const bool         default_landfall           = false;
static const int          default_landfall_beg       = -86400;
static const int          default_landfall_end       = 0;

// Default alpha value and FSP significance threshold
static const double       default_tc_alpha           = 0.05;
static const SingleThresh default_fsp_thresh(">0");

////////////////////////////////////////////////////////////////////////

// Define struct to store the mapped StringArray and NumArray values
struct SummaryMapData {
   NumArray    Val;
   StringArray Hdr;
   StringArray AModel;
   TimeArray   Init;
   NumArray    Lead;
   TimeArray   Valid;
};

////////////////////////////////////////////////////////////////////////

// Define struct to store the mapped StringArray and RIRW contingency
// table counts and statistics
struct RIRWMapData {
   CTSInfo     Info;
   StringArray Hdr;
};

////////////////////////////////////////////////////////////////////////

// Define struct to store the mapped StringArray and ProbRIRW
// probabilistic contingency table counts and statistics
struct ProbRIRWMapData {
   PCTInfo     Info;
   int         RIRWWindow;
   StringArray Hdr;
};

////////////////////////////////////////////////////////////////////////

// Define struct used to perform comparisons on ConcatStrings
struct cs_cmp {
  bool operator()(const ConcatString & cs1, const ConcatString & cs2) const {
    return(cs1 < cs2);
  }
};

////////////////////////////////////////////////////////////////////////

//
// Enumerate all the possible TC-STAT Analysis Job Types
//
enum TCStatJobType {

   TCStatJobType_Filter,   // Filter out the TC-STAT data and write
                           // the lines to the filename specified.
   TCStatJobType_Summary,  // Compute summary info for one or more
                           // columns of data.
   TCStatJobType_RIRW,     // Derive contingency table and statistics
                           // for RI/RW events.
   TCStatJobType_ProbRIRW, // Derive probabilistic contingency table and
                           // statistics for PROBRIRW lines.
   NoTCStatJobType         // Default value
};

extern TCStatJobType string_to_tcstatjobtype(const ConcatString);
extern ConcatString  tcstatjobtype_to_string(const TCStatJobType);

////////////////////////////////////////////////////////////////////////

// Struct for counts of lines read and rejected
struct TCLineCounts {

   // Read and keep counts
   int NRead;
   int NKeep;

   // Checking entire track
   int RejTrackWatchWarn;
   int RejInitThresh;
   int RejInitStr;

   // Filtering on track attributes
   int RejRIRW;
   int RejLandfall;

   // Checking track point attributes
   int RejAModel;
   int RejBModel;
   int RejDesc;
   int RejStormId;
   int RejBasin;
   int RejCyclone;
   int RejStormName;
   int RejInit;
   int RejInitHour;
   int RejLead;
   int RejValid;
   int RejValidHour;
   int RejInitMask;
   int RejValidMask;
   int RejLineType;
   int RejWaterOnly;
   int RejColumnThresh;
   int RejColumnStr;
   int RejMatchPoints;
   int RejEventEqual;
   int RejOutInitMask;
   int RejOutValidMask;
   int RejLeadReq;
};

////////////////////////////////////////////////////////////////////////

// Forward decalartion
class TCStatJob;

class TCStatJobFactory {
   public:
      static TCStatJob *new_tc_stat_job_type(const char *type_str);
      static TCStatJob *new_tc_stat_job(const char *jobstring);
};

////////////////////////////////////////////////////////////////////////

class TCStatJob {

   protected:

      void init_from_scratch();

      void assign(const TCStatJob &);

      // Output file precision
      int Precision;

   public:

      TCStatJob();
      virtual ~TCStatJob();
      TCStatJob(const TCStatJob &);
      TCStatJob & operator=(const TCStatJob &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      //////////////////////////////////////////////////////////////////

      void set_precision (int);

      int  get_precision () const;

      //////////////////////////////////////////////////////////////////

      bool is_keeper_track(const TrackPairInfo &, TCLineCounts &) const;

      bool is_keeper_line(const TCStatLine &, TCLineCounts &) const;

      double get_column_double(const TCStatLine &, const ConcatString &) const;

      //////////////////////////////////////////////////////////////////

      virtual StringArray parse_job_command(const char *);

      void set_out_init_mask(const char *);
      void set_out_valid_mask(const char *);

      void open_dump_file();
      void close_dump_file();

      void open_stat_file();
      void close_stat_file();

      void dump_pair(const TrackPairInfo &, ofstream *);
      void dump_line(const TCStatLine &,    ofstream *);

      virtual ConcatString serialize() const;

      //////////////////////////////////////////////////////////////////

      virtual void do_job(const StringArray &, TCLineCounts &);

      void event_equalize_tracks();

      void event_equalize_lines();

      void subset_track_pair(TrackPairInfo &,  TCLineCounts &);

      //////////////////////////////////////////////////////////////////

      // Interface with the input data files for this job
      TCStatFiles TCSTFiles;

      // Job Type
      TCStatJobType JobType;

      // Variables to stratify the input TC-STAT lines

      // Header column entries
      StringArray AModel;
      StringArray BModel;
      StringArray Desc;
      StringArray StormId;
      StringArray Basin;
      StringArray Cyclone;
      StringArray StormName;

      // Initialization Times
      unixtime  InitBeg, InitEnd;
      TimeArray InitInc;
      TimeArray InitExc;
      NumArray  InitHour;
      NumArray  Lead;
      NumArray  LeadReq;

      // Valid Times
      unixtime  ValidBeg, ValidEnd;
      TimeArray ValidInc;
      TimeArray ValidExc;
      NumArray  ValidHour;

      // Polyline masking regions
      StringArray InitMask, ValidMask;

      // Track watch/warning status
      StringArray TrackWatchWarn;

      // Line type
      StringArray LineType;

      // Only retain TrackPoints where ADLAND and BDLAND > 0
      bool WaterOnly;

      // Numeric column thresholds
      map<ConcatString,ThreshArray> ColumnThreshMap;

      // ASCII column string matching
      map<ConcatString,StringArray> ColumnStrMap;

      // Numeric column thresholds
      map<ConcatString,ThreshArray> InitThreshMap;

      // ASCII column string matching
      map<ConcatString,StringArray> InitStrMap;

      // Variables to the store the analysis job specification
      ConcatString DumpFile;        // Dump TrackPairInfo used to a file
      ofstream    *DumpOut;         // Dump output file stream
      ofstream    *JobOut;          // Job output file stream (not allocated)

      // Derived output statistics
      ConcatString StatFile;        // File name for output statistics
      ofstream    *StatOut;         // Output statistics file stream

      // Polyline masking regions
      ConcatString OutInitMaskFile;
      ConcatString OutInitMaskName;
      MaskPoly     OutInitPolyMask;
      Grid         OutInitGridMask;
      MaskPlane    OutInitAreaMask;

      ConcatString OutValidMaskFile;
      ConcatString OutValidMaskName;
      MaskPoly     OutValidPolyMask;
      Grid         OutValidGridMask;
      MaskPlane    OutValidAreaMask;

      // Only retain TrackPoints in both the ADECK and BDECK tracks
      bool MatchPoints;

      // Only retain cases present for all models
      bool        EventEqual;
      bool        EventEqualSet;
      NumArray    EventEqualLead;
      StringArray EventEqualCases;

      // Rapid intensification/weakening logic for each track
      TrackType    RIRWTrack;
      int          RIRWTimeADeck, RIRWTimeBDeck;
      bool         RIRWExactADeck, RIRWExactBDeck;
      SingleThresh RIRWThreshADeck, RIRWThreshBDeck;
      int          RIRWWindowBeg;
      int          RIRWWindowEnd;

      // ProbRIRW filtering logic
      //   e.g. -30, -10, 0, 10, 30, 55, 65
      double       ProbRIRWThresh;

      // Only retain TrackPoints in a time window around landfall
      bool         Landfall;
      int          LandfallBeg;
      int          LandfallEnd;
};

////////////////////////////////////////////////////////////////////////

inline void TCStatJob::set_precision (int p)  { Precision = p; return; }
inline int  TCStatJob::get_precision () const { return(Precision);     }

////////////////////////////////////////////////////////////////////////

class TCStatJobFilter : public TCStatJob {

   private:

      void init_from_scratch();

      void assign(const TCStatJobFilter &);

   public:

      TCStatJobFilter();
      virtual ~TCStatJobFilter();
      TCStatJobFilter(const TCStatJobFilter &);
      TCStatJobFilter & operator=(const TCStatJobFilter &);

      void clear();

      void do_job(const StringArray &, TCLineCounts &); // virtual from base class

      void filter_tracks(TCLineCounts &);
      void filter_lines (TCLineCounts &);

      void do_output(ostream &);

};

////////////////////////////////////////////////////////////////////////

class TCStatJobSummary : public TCStatJob {

   private:

      void init_from_scratch();

      void assign(const TCStatJobSummary &);

   public:

      TCStatJobSummary();
      virtual ~TCStatJobSummary();
      TCStatJobSummary(const TCStatJobSummary &);
      TCStatJobSummary & operator=(const TCStatJobSummary &);

      void clear();

      StringArray parse_job_command(const char *);

      void add_column(const char *);

      ConcatString serialize() const;

      void do_job(const StringArray &, TCLineCounts &); // virtual from base class

      void summarize_tracks(TCLineCounts &);
      void summarize_lines (TCLineCounts &);

      void process_pair(TrackPairInfo &);
      void process_line(TCStatLine &);

      void add_map(map<ConcatString,SummaryMapData,cs_cmp>&);

      void do_output(ostream &);

      void compute_fsp(NumArray &, NumArray &, NumArray &);

      // Store the requested column names
      StringArray ReqColumn;

      // Store the actual column names
      StringArray Column;
      bool ColumnUnion;

      // Store the case information
      StringArray ByColumn;

      // Confidence interval alpha value
      double OutAlpha;

      // Threshold to determine meaningful improvements
      SingleThresh FSPThresh;

      // Map column and case info to column values
      map<ConcatString,SummaryMapData,cs_cmp> SummaryMap;

};

////////////////////////////////////////////////////////////////////////

class TCStatJobRIRW : public TCStatJob {

   private:

      void init_from_scratch();

      void assign(const TCStatJobRIRW &);

      ConcatString DumpFileCTC[4];
      ofstream    *DumpOutCTC[4];

   public:

      TCStatJobRIRW();
      virtual ~TCStatJobRIRW();
      TCStatJobRIRW(const TCStatJobRIRW &);
      TCStatJobRIRW & operator=(const TCStatJobRIRW &);

      void clear();

      StringArray parse_job_command(const char *);

      void open_dump_file();
      void close_dump_file();

      ConcatString serialize() const;

      void do_job(const StringArray &, TCLineCounts &); // virtual from base class

      void process_pair(TrackPairInfo &);

      void add_map(map<ConcatString,RIRWMapData,cs_cmp>&);

      void do_output    (ostream &);
      void do_ctc_output(ostream &);
      void do_cts_output(ostream &);
      void do_mpr_output(ostream &);

      // Store the case information
      StringArray ByColumn;

      // Confidence interval alpha value
      double OutAlpha;

      // Output types
      StringArray OutLineType;

      // Map column and case info to column values
      map<ConcatString,RIRWMapData,cs_cmp> RIRWMap;

};

////////////////////////////////////////////////////////////////////////

class TCStatJobProbRIRW : public TCStatJob {

   private:

      void init_from_scratch();

      void assign(const TCStatJobProbRIRW &);

   public:

      TCStatJobProbRIRW();
      virtual ~TCStatJobProbRIRW();
      TCStatJobProbRIRW(const TCStatJobProbRIRW &);
      TCStatJobProbRIRW & operator=(const TCStatJobProbRIRW &);

      void clear();

      StringArray parse_job_command(const char *);

      void close_dump_file();

      ConcatString serialize() const;

      void do_job(const StringArray &, TCLineCounts &); // virtual from base class

      void process_pair(ProbRIRWPairInfo &);

      void do_output(ostream &);

      // Probability information
      bool         ProbRIRWExact;        // True for exact change, false for maximum change
      SingleThresh ProbRIRWBDeltaThresh; // Threshold the BEST track change
      ThreshArray  ProbRIRWProbThresh;   // Array of probabilities for PCT bins

      // Store the case information
      StringArray ByColumn;

      // Maximum number of thresholds encountered
      int MaxNThresh;
      int NDumpLines;

      // Confidence interval alpha value
      double OutAlpha;

      // Output types
      StringArray OutLineType;

      // Map column and case info to column values
      map<ConcatString,ProbRIRWMapData,cs_cmp> ProbRIRWMap;

};

////////////////////////////////////////////////////////////////////////

extern bool        is_time_series(const TimeArray &, const NumArray &,
                                  const TimeArray &, int &);
extern int         compute_time_to_indep(const NumArray &, int);
extern StringArray intersection(const StringArray &, const StringArray &);
extern double      get_probrirw_value(const TCStatLine &, double);

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_STAT_JOB_H__  */

////////////////////////////////////////////////////////////////////////
