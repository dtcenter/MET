// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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

#include "mask_poly.h"
#include "vx_tc_util.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

// Defaults to be used if not specified by the user
static const bool         default_water_only         = false;
static const bool         default_match_points       = false;

// Default rapid intensification is an increase of 30 kts
static const bool         default_rapid_inten        = false;
static const SingleThresh default_rapid_inten_thresh(">=30.0");

// Default is 24 hours prior to landfall
static const bool         default_landfall           = false;
static const int          default_landfall_beg       = -86400;
static const int          default_landfall_end       = 0;

static const double       default_tc_alpha           = 0.05;
static const SingleThresh default_fsp_thresh(">0");

////////////////////////////////////////////////////////////////////////

// Define struct to store the mapped StringArray and NumArray values
struct MapData {
   NumArray    Val;
   StringArray Hdr;
   StringArray AModel;
   TimeArray   Init;
   NumArray    Lead;
   TimeArray   Valid;
};

void clear_map_data(MapData &);

////////////////////////////////////////////////////////////////////////

// Define struct used to perform comparisons on ConcatStrings
struct cs_cmp {
  bool operator()(const ConcatString & cs1, const ConcatString & cs2) const {
    return(strcmp(cs1, cs2) < 0);
  }
};

////////////////////////////////////////////////////////////////////////

//
// Enumerate all the possible TC-STAT Analysis Job Types
//
enum TCStatJobType {

   TCStatJobType_Filter,  // Filter out the TC-STAT data and write
                          // the lines to the filename specified.
   TCStatJobType_Summary, // Compute summary info for one or more
                          // columns of data.
   NoTCStatJobType        // Default value
};

extern TCStatJobType string_to_tcstatjobtype(const char *);
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
   int RejRapidInten;
   int RejLandfall;

   // Checking track point attributes
   int RejAModel;
   int RejBModel;
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
   int RejOutInitMask;
   int RejOutValidMask;
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
      
   public:

      TCStatJob();
     ~TCStatJob();
      TCStatJob(const TCStatJob &);
      TCStatJob & operator=(const TCStatJob &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      bool is_keeper_track(const TrackPairInfo &,
                           TCLineCounts &) const;

      void filter_track(TrackPairInfo &,
                        TCLineCounts &) const;

      bool is_keeper_line(const TCStatLine &,
                          TCLineCounts &) const;

      double get_column_double(const TCStatLine &,
                               const ConcatString &) const;

      virtual StringArray parse_job_command(const char *);
      void set_mask(MaskPoly &, const char *);
      void open_dump_file();
      void close_dump_file();
      void write_dump_file();      

      virtual ConcatString serialize() const;

      virtual void do_job(const StringArray &, TCLineCounts &);

      virtual void process_tc_stat_file(const char *, TCLineCounts &);

      void process_track_pair(TrackPairInfo &, TCLineCounts &);
      
      // Job Type
      TCStatJobType JobType;

      // Array to store the filtered TrackPairInfo
      TrackPairInfoArray PairArray;

      // Variables to stratify the input TC-STAT lines

      // Header column entries
      StringArray AModel;
      StringArray BModel;
      StringArray StormId;
      StringArray Basin;
      StringArray Cyclone;
      StringArray StormName;

      // Timing information
      unixtime  InitBeg, InitEnd;
      TimeArray InitExc;
      NumArray  InitHour;
      NumArray  Lead;
      unixtime  ValidBeg, ValidEnd;
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
      StringArray ColumnThreshName;
      ThreshArray ColumnThreshVal;

      // ASCII column string matching
      StringArray ColumnStrName;
      StringArray ColumnStrVal;

      // Numeric column thresholds
      StringArray InitThreshName;
      ThreshArray InitThreshVal;

      // ASCII column string matching
      StringArray InitStrName;
      StringArray InitStrVal;
      
      // Variables to the store the analysis job specification
      
      ConcatString DumpFile;        // Dump TrackPairInfo used to a file
      ofstream    *DumpOut;         // Dump output file stream
      ofstream    *JobOut;          // Job output file stream (not allocated)

      MaskPoly     OutInitMask;     // Polyline masking region
      MaskPoly     OutValidMask;    // Polyline masking region

      // Only retain TrackPoints in both the ADECK and BDECK tracks
      bool MatchPoints;

      // Only retain TrackPoints with recent rapid intensification
      bool         RapidInten;
      SingleThresh RapidIntenThresh;

      // Only retain TrackPoints in a time window around landfall
      bool Landfall;
      int  LandfallBeg;
      int  LandfallEnd;
};

////////////////////////////////////////////////////////////////////////

class TCStatJobFilter : public TCStatJob {

   private:

      void init_from_scratch();

      void assign(const TCStatJobFilter &);

   public:

      TCStatJobFilter();
     ~TCStatJobFilter();
      TCStatJobFilter(const TCStatJobFilter &);
      TCStatJobFilter & operator=(const TCStatJobFilter &);

      void clear();

      void do_job(const StringArray &, TCLineCounts &); // virtual from base class

      void do_output(ostream &);
      
};

////////////////////////////////////////////////////////////////////////

class TCStatJobSummary : public TCStatJob {

   private:

      void init_from_scratch();

      void assign(const TCStatJobSummary &);

   public:

      TCStatJobSummary();
     ~TCStatJobSummary();
      TCStatJobSummary(const TCStatJobSummary &);
      TCStatJobSummary & operator=(const TCStatJobSummary &);

      void clear();

      StringArray parse_job_command(const char *);
      
      void add_column(const char *);

      ConcatString serialize() const;
      
      void do_job(const StringArray &, TCLineCounts &); // virtual from base class

      void process_pair_array();
      
      void add_map(map<ConcatString,MapData,cs_cmp>&);

      void do_output(ostream &);

      void compute_fsp(NumArray &, NumArray &, NumArray &);
      
      // Store the requested column names
      StringArray ReqColumn;

      // Store the actual column names
      StringArray Column;

      // Store the case information
      StringArray Case;

      // Confidence interval alpha value
      double OutAlpha;

      // Threshold to determine meaningful improvements
      SingleThresh FSPThresh;

      // Map column and case info to column values
      map<ConcatString,MapData,cs_cmp> SummaryMap;

};

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_STAT_JOB_H__  */

////////////////////////////////////////////////////////////////////////
