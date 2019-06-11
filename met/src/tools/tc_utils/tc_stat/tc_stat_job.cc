// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <cmath>
#include <map>
#include <utility>

#include "tc_stat_job.h"

#include "met_stats.h"
#include "apply_mask.h"
#include "vx_tc_util.h"
#include "vx_log.h"
#include "vx_util.h"
#include "vx_math.h"
#include "vx_stat_out.h"

////////////////////////////////////////////////////////////////////////

static const char *TCStatJobType_FilterStr   = "filter";
static const char *TCStatJobType_SummaryStr  = "summary";
static const char *TCStatJobType_RIRWStr     = "rirw";
static const char *TCStatJobType_ProbRIRWStr = "probrirw";

////////////////////////////////////////////////////////////////////////

// Functions for parsing command line options
static void         parse_thresh_option(const char *, const char *, map<ConcatString,ThreshArray> &);
static void         parse_string_option(const char *, const char *, map<ConcatString,StringArray> &);
static void         setup_table        (AsciiTable &, int, int);
static ConcatString build_map_key      (const char *, const TCStatLine &, const StringArray &);
static bool         check_masks        (const MaskPoly &, const Grid &, const MaskPlane &,
                                        double lat, double lon);

////////////////////////////////////////////////////////////////////////
//
// Code for class TCStatJobFactory
//
////////////////////////////////////////////////////////////////////////

TCStatJob *TCStatJobFactory::new_tc_stat_job_type(const char *type_str) {
   TCStatJob *job = (TCStatJob *) 0;
   TCStatJobType type = NoTCStatJobType;

   // Determine the TCStatJobType
   type = string_to_tcstatjobtype((string)type_str);

   // Switch on job type and instantiate the appropriate class.
   // The TCStatJob object is allocated and needs to be deleted by caller.
   switch(type) {

      case TCStatJobType_Filter:
         job = new TCStatJobFilter;
         break;

      case TCStatJobType_Summary:
         job = new TCStatJobSummary;
         break;

      case TCStatJobType_RIRW:
         job = new TCStatJobRIRW;
         break;

      case TCStatJobType_ProbRIRW:
         job = new TCStatJobProbRIRW;
         break;

      case NoTCStatJobType:
      default:
         mlog << Error << "\nTCStatJobFactory::new_tc_stat_job_type() -> "
              << "unsupported job type \"" << type_str << "\"\n\n";
         exit(1);
         break;
   } // end switch

   return(job);
}

///////////////////////////////////////////////////////////////////////////////

TCStatJob *TCStatJobFactory::new_tc_stat_job(const char *jobstring) {
   TCStatJob *job = (TCStatJob *) 0;
   StringArray a;
   ConcatString type_str = na_string;
   ConcatString err_str;
   int i;

   // Parse the jobstring into an array
   a.parse_wsss(jobstring);

   // Check if the job type is specified
   if(a.has("-job", i)) type_str = a[i+1];

   // Allocate a new job of the requested type
   job = new_tc_stat_job_type(type_str.c_str());

   // Parse the jobstring into the new job
   a = job->parse_job_command(jobstring);

   // Check for unused arguments
   if(a.n_elements() > 0) {

      // Build list of unknown args
      for(i=0; i<a.n_elements()-1; i++) err_str << a[i] << " ";
      err_str << a[a.n_elements()-1];

      mlog << Error
           << "\nTCStatJob *TCStatJobFactory::new_tc_stat_job(const char *jobstring) -> "
           << "unsupported job command options \""
           << err_str << "\".\n\n";
      exit(1);
   }

   return(job);
}

////////////////////////////////////////////////////////////////////////
//
// Code for class TCStatJob
//
////////////////////////////////////////////////////////////////////////

TCStatJob::TCStatJob() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCStatJob::~TCStatJob() {

   clear();
}

////////////////////////////////////////////////////////////////////////

TCStatJob::TCStatJob(const TCStatJob &j) {

   init_from_scratch();

   assign(j);
}

////////////////////////////////////////////////////////////////////////

TCStatJob & TCStatJob::operator=(const TCStatJob &j) {

   if(this == &j) return(*this);

   assign(j);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::init_from_scratch() {

   DumpOut = (ofstream *) 0;
   JobOut  = (ofstream *) 0;
   StatOut = (ofstream *) 0;

   // Ignore case when performing comparisons
   AModel.set_ignore_case(1);
   BModel.set_ignore_case(1);
   StormId.set_ignore_case(1);
   Basin.set_ignore_case(1);
   Cyclone.set_ignore_case(1);
   StormName.set_ignore_case(1);
   InitMask.set_ignore_case(1);
   ValidMask.set_ignore_case(1);
   LineType.set_ignore_case(1);
   TrackWatchWarn.set_ignore_case(1);

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::clear() {

   Precision = default_precision;

   JobType = NoTCStatJobType;

   AModel.clear();
   BModel.clear();
   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();
   InitBeg = InitEnd = (unixtime) 0;
   InitInc.clear();
   InitExc.clear();
   InitHour.clear();
   LeadReq.clear();
   Lead.clear();
   ValidBeg = ValidEnd = (unixtime) 0;
   ValidInc.clear();
   ValidExc.clear();
   ValidHour.clear();
   InitMask.clear();
   ValidMask.clear();
   LineType.clear();
   TrackWatchWarn.clear();
   ColumnThreshMap.clear();
   ColumnStrMap.clear();
   InitThreshMap.clear();
   InitStrMap.clear();
   EventEqualLead.clear();
   EventEqualCases.clear();

   DumpFile.clear();
   close_dump_file();
   JobOut = (ofstream *) 0;

   StatFile.clear();
   close_stat_file();
   StatOut = (ofstream *) 0;

   // Set to default values
   WaterOnly       = default_water_only;
   RIRWTrack       = default_rirw_track;
   RIRWTimeADeck   = default_rirw_time;
   RIRWTimeBDeck   = default_rirw_time;
   RIRWExactADeck  = default_rirw_exact;
   RIRWExactBDeck  = default_rirw_exact;
   RIRWThreshADeck = default_rirw_thresh;
   RIRWThreshBDeck = default_rirw_thresh;
   RIRWWindowBeg   = default_rirw_window;
   RIRWWindowEnd   = default_rirw_window;
   ProbRIRWThresh  = default_probrirw_thresh;
   Landfall        = default_landfall;
   LandfallBeg     = default_landfall_beg;
   LandfallEnd     = default_landfall_end;
   MatchPoints     = default_match_points;
   EventEqual      = default_event_equal;
   EventEqualSet   = false;

   OutInitMaskFile.clear();
   OutInitMaskName.clear();
   OutInitPolyMask.clear();
   OutInitGridMask.clear();
   OutInitAreaMask.clear();

   OutValidMaskFile.clear();
   OutValidMaskName.clear();
   OutValidPolyMask.clear();
   OutValidGridMask.clear();
   OutValidAreaMask.clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::assign(const TCStatJob & j) {

   clear();

   Precision = j.Precision;

   JobType = j.JobType;

   AModel = j.AModel;
   BModel = j.BModel;
   StormId = j.StormId;
   Basin = j.Basin;
   Cyclone = j.Cyclone;
   StormName = j.StormName;
   InitBeg = j.InitBeg;
   InitEnd = j.InitEnd;
   InitInc = j.InitInc;
   InitExc = j.InitExc;
   InitHour = j.InitHour;
   Lead = j.Lead;
   LeadReq = j.LeadReq;
   ValidBeg = j.ValidBeg;
   ValidEnd = j.ValidEnd;
   ValidInc = j.ValidInc;
   ValidExc = j.ValidExc;
   ValidHour = j.ValidHour;
   InitMask = j.InitMask;
   ValidMask = j.ValidMask;
   LineType = j.LineType;
   TrackWatchWarn = j.TrackWatchWarn;
   ColumnThreshMap = j.ColumnThreshMap;
   ColumnStrMap = j.ColumnStrMap;
   InitThreshMap = j.InitThreshMap;
   InitStrMap = j.InitStrMap;

   DumpFile = j.DumpFile;
   open_dump_file();

   StatFile = j.StatFile;
   open_stat_file();

   WaterOnly = j.WaterOnly;

   RIRWTrack = j.RIRWTrack;
   RIRWTimeADeck = j.RIRWTimeADeck;
   RIRWTimeBDeck = j.RIRWTimeBDeck;
   RIRWExactADeck = j.RIRWExactADeck;
   RIRWExactBDeck = j.RIRWExactBDeck;
   RIRWThreshADeck = j.RIRWThreshADeck;
   RIRWThreshBDeck = j.RIRWThreshBDeck;
   RIRWWindowBeg = j.RIRWWindowBeg;
   RIRWWindowEnd = j.RIRWWindowEnd;

   ProbRIRWThresh = j.ProbRIRWThresh;

   Landfall = j.Landfall;
   LandfallBeg = j.LandfallBeg;
   LandfallEnd = j.LandfallEnd;

   MatchPoints = j.MatchPoints;
   EventEqual = j.EventEqual;
   EventEqualSet = j.EventEqualSet;
   EventEqualLead = j.EventEqualLead;
   EventEqualCases = j.EventEqualCases;

   OutInitMaskFile = j.OutInitMaskFile;
   OutInitMaskName = j.OutInitMaskName;
   OutInitPolyMask = j.OutInitPolyMask;
   OutInitGridMask = j.OutInitGridMask;
   OutInitAreaMask = j.OutInitAreaMask;

   OutValidMaskFile = j.OutValidMaskFile;
   OutValidMaskName = j.OutValidMaskName;
   OutValidPolyMask = j.OutValidPolyMask;
   OutValidGridMask = j.OutValidGridMask;
   OutValidAreaMask = j.OutValidAreaMask;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::dump(ostream & out, int depth) const {
   Indent prefix(depth);
   map<ConcatString,ThreshArray>::const_iterator thr_it;
   map<ConcatString,StringArray>::const_iterator str_it;

   out << prefix << "JobType = " << tcstatjobtype_to_string(JobType)
       << "\n";

   out << prefix << "AModel ...\n";
   AModel.dump(out, depth + 1);

   out << prefix << "BModel ...\n";
   BModel.dump(out, depth + 1);

   out << prefix << "StormId ...\n";
   StormId.dump(out, depth + 1);

   out << prefix << "Basin ...\n";
   Basin.dump(out, depth + 1);

   out << prefix << "Cyclone ...\n";
   Cyclone.dump(out, depth + 1);

   out << prefix << "StormName ...\n";
   StormName.dump(out, depth + 1);

   out << prefix << "InitBeg = " << unix_to_yyyymmdd_hhmmss(InitBeg) << "\n";
   out << prefix << "InitEnd = " << unix_to_yyyymmdd_hhmmss(InitEnd) << "\n";

   out << prefix << "InitInc ...\n";
   InitInc.dump(out, depth + 1);

   out << prefix << "InitExc ...\n";
   InitExc.dump(out, depth + 1);

   out << prefix << "InitHour ...\n";
   InitHour.dump(out, depth + 1);

   out << prefix << "Lead ...\n";
   Lead.dump(out, depth + 1);

   out << prefix << "ValidBeg = " << unix_to_yyyymmdd_hhmmss(ValidBeg) << "\n";
   out << prefix << "ValidEnd = " << unix_to_yyyymmdd_hhmmss(ValidEnd) << "\n";

   out << prefix << "ValidInc ...\n";
   ValidInc.dump(out, depth + 1);

   out << prefix << "ValidExc ...\n";
   ValidExc.dump(out, depth + 1);

   out << prefix << "ValidHour ...\n";
   ValidHour.dump(out, depth + 1);

   out << prefix << "InitMask ...\n";
   InitMask.dump(out, depth + 1);

   out << prefix << "ValidMask ...\n";
   ValidMask.dump(out, depth + 1);

   out << prefix << "LineType ...\n";
   LineType.dump(out, depth + 1);

   out << prefix << "TrackWatchWarn ...\n";
   TrackWatchWarn.dump(out, depth + 1);

   out << prefix << "ColumnThreshMap ...\n";
   for(thr_it=ColumnThreshMap.begin(); thr_it!= ColumnThreshMap.end(); thr_it++) {
      out << prefix << thr_it->first << ": \n";
      thr_it->second.dump(out, depth + 1);
   }

   out << prefix << "ColumnStrMap ...\n";
   for(str_it=ColumnStrMap.begin(); str_it!= ColumnStrMap.end(); str_it++) {
      out << prefix << str_it->first << ": \n";
      str_it->second.dump(out, depth + 1);
   }

   out << prefix << "InitThreshMap ...\n";
   for(thr_it=InitThreshMap.begin(); thr_it!= InitThreshMap.end(); thr_it++) {
      out << prefix << thr_it->first << ": \n";
      thr_it->second.dump(out, depth + 1);
   }

   out << prefix << "InitStrMap ...\n";
   for(str_it=InitStrMap.begin(); str_it!= InitStrMap.end(); str_it++) {
      out << prefix << str_it->first << ": \n";
      str_it->second.dump(out, depth + 1);
   }

   out << prefix << "WaterOnly = " << bool_to_string(WaterOnly) << "\n";

   out << prefix << "RIRWTrack = " << tracktype_to_string(RIRWTrack) << "\n";

   out << prefix << "RIRWTimeADeck = " << sec_to_hhmmss(RIRWTimeADeck) << "\n";

   out << prefix << "RIRWTimeBDeck = " << sec_to_hhmmss(RIRWTimeBDeck) << "\n";

   out << prefix << "RIRWExactADeck = " << bool_to_string(RIRWExactADeck) << "\n";

   out << prefix << "RIRWExactBDeck = " << bool_to_string(RIRWExactBDeck) << "\n";

   out << prefix << "RIRWThreshADeck = " << RIRWThreshADeck.get_str() << "\n";

   out << prefix << "RIRWThreshBDeck = " << RIRWThreshBDeck.get_str() << "\n";

   out << prefix << "RIRWWindowBeg = " << sec_to_hhmmss(RIRWWindowBeg) << "\n";

   out << prefix << "RIRWWindowEnd = " << sec_to_hhmmss(RIRWWindowEnd) << "\n";

   out << prefix << "ProbRIRWThresh = " << ProbRIRWThresh << "\n";

   out << prefix << "Landfall = " << bool_to_string(Landfall) << "\n";

   out << prefix << "LandfallBeg = " << sec_to_hhmmss(LandfallBeg) << "\n";

   out << prefix << "LandfallEnd = " << sec_to_hhmmss(LandfallEnd) << "\n";

   out << prefix << "MatchPoints = " << bool_to_string(MatchPoints) << "\n";

   out << prefix << "EventEqual = " << bool_to_string(EventEqual) << "\n";

   out << prefix << "EventEqualLead ...\n";
   EventEqualLead.dump(out, depth + 1);

   out << prefix << "OutInitMask = " << (OutInitMaskName.nonempty() ? OutInitMaskName.text() : na_str) << "\n";

   out << prefix << "OutValidMask = " << (OutValidMaskName.nonempty() ? OutValidMaskName.text() : na_str) << "\n";

   out << prefix << "DumpFile = " << (DumpFile.nonempty() ? DumpFile.text() : na_str) << "\n";

   out << prefix << "StatFile = " << (StatFile.nonempty() ? StatFile.text() : na_str) << "\n";

   out.flush();

   return;
}

////////////////////////////////////////////////////////////////////////

bool TCStatJob::is_keeper_track(const TrackPairInfo &pair,
                                TCLineCounts &n) const {
   bool keep = true;
   int i, i_init;
   double v_dbl;
   ConcatString v_str;
   map<ConcatString,ThreshArray>::const_iterator thr_it;
   map<ConcatString,StringArray>::const_iterator str_it;

   // Check TrackWatchWarn for each TrackPoint
   if(TrackWatchWarn.n_elements() > 0) {

      // Assume track will not be kept
      keep = false;

      // Loop through the points to see if any are to be kept
      for(i=0; i<pair.n_points(); i++) {

         // Get the current watch/warning type
         WatchWarnType ww_type = pair.watch_warn(i);

         // Check for the current watch/warn status in the list and the
         // special string ALL:
         //    HUWARN, TSWARN, HUWATCH, TSWATCH
         if(TrackWatchWarn.has(watchwarntype_to_string(ww_type)) ||
            (TrackWatchWarn.has("ALL") &&
             (ww_type == HurricaneWarn     ||
              ww_type == TropicalStormWarn ||
              ww_type == HurricaneWatch    ||
              ww_type == TropicalStormWatch))) {
            keep = true;
            break;
         }
      } // end for i
   } // end if

   // Update counts
   if(!keep) n.RejTrackWatchWarn += pair.n_points();

   // Get the index of the track initialization point
   i_init=pair.i_init();

   // Check for bad track initialization point
   if(is_bad_data(i_init)) {
      if(InitThreshMap.size() > 0) {
         keep = false;
         n.RejInitThresh += pair.n_points();
      }
      else if(InitStrMap.size() > 0) {
         keep = false;
         n.RejInitStr += pair.n_points();
      }
      else if(OutInitMaskName.nonempty()) {
         keep = false;
         n.RejOutInitMask += pair.n_points();
      }
   }

   // Check InitThreshMap
   if(keep == true) {

      // Loop through the numeric column thresholds
      for(thr_it=InitThreshMap.begin(); thr_it!= InitThreshMap.end(); thr_it++) {

         // Get the numeric init column value
         v_dbl = get_column_double(*pair.line(i_init), thr_it->first);

         // Check the column threshold
         if(!thr_it->second.check_dbl(v_dbl)) {
           keep = false;
           n.RejInitThresh += pair.n_points();
           break;
         }
      }
   }

   // Check InitStr
   if(keep == true) {

      for(str_it=InitStrMap.begin(); str_it!= InitStrMap.end(); str_it++) {

         // Retrieve the column value
         v_str = pair.line(i_init)->get_item(str_it->first.c_str());

         // Check the string value
         if(!str_it->second.has(v_str)) {
            keep = false;
            n.RejInitStr += pair.n_points();
            break;
         }
      }
   }

   // Check OutInitMask
   if(keep == true) {

      // A mask has been specified
      if(OutInitMaskName.nonempty()) {

         // Check if ADECK init location falls inside the mask
         if(is_bad_data(pair.adeck()[i_init].lat()) ||
            is_bad_data(pair.adeck()[i_init].lon()) ||
            !check_masks(OutInitPolyMask,
                         OutInitGridMask,
                         OutInitAreaMask,
                         pair.adeck()[i_init].lat(),
                         pair.adeck()[i_init].lon())) {
            keep = false;
            n.RejOutInitMask += pair.n_points();
         }
      }
   }

   // MET-667 Check this track for required lead times
   // If no required lead times were defined, do nothing.
   if(keep == true && LeadReq.n_elements() > 0){
      // Loop through the points and see if any of the
      // lead times are in the list of required lead times
      // defined in the configuration file.
      for(int j=0; j<LeadReq.n_elements(); j++){
         if(pair.adeck().lead_index(LeadReq[j]) == -1){
            // If we don't have a match, break out and discard this track.
            keep = false;
            n.RejLeadReq += pair.n_points();
            break;
         }
      }
   }

   // Update counts
   if(!keep) n.NKeep -= pair.n_points();

   return(keep);
}

////////////////////////////////////////////////////////////////////////

bool TCStatJob::is_keeper_line(const TCStatLine &line,
                               TCLineCounts &n) const {
   bool keep = true;
   double v_dbl, alat, alon, blat, blon;
   ConcatString v_str;
   StringArray sa;
   map<ConcatString,ThreshArray>::const_iterator thr_it;
   map<ConcatString,StringArray>::const_iterator str_it;

   // Check TC-STAT header columns
   if(AModel.n_elements() > 0 &&
     !AModel.has(line.amodel()))        { keep = false; n.RejAModel++;    }
   else if(BModel.n_elements() > 0 &&
     !BModel.has(line.bmodel()))        { keep = false; n.RejBModel++;    }
   else if(Desc.n_elements() > 0 &&
     !Desc.has(line.desc()))            { keep = false; n.RejDesc++;      }
   else if(StormId.n_elements() > 0 &&
   !has_storm_id(StormId, (string)line.basin(), (string)line.cyclone(), line.init()))
                                        { keep = false; n.RejStormId++;   }
   else if(Basin.n_elements() > 0 &&
     !Basin.has(line.basin()))          { keep = false; n.RejBasin++;     }
   else if(Cyclone.n_elements() > 0 &&
     !Cyclone.has(line.cyclone()))      { keep = false; n.RejCyclone++;   }
   else if(StormName.n_elements() > 0 &&
     !StormName.has(line.storm_name())) { keep = false; n.RejStormName++; }
   else if(InitBeg > 0 &&
      line.init() < InitBeg)            { keep = false; n.RejInit++;      }
   else if(InitEnd > 0 &&
      line.init() > InitEnd)            { keep = false; n.RejInit++;      }
   else if(InitInc.n_elements() > 0 &&
     !InitInc.has(line.init()))         { keep = false; n.RejInit++;      }
   else if(InitExc.n_elements() > 0 &&
     InitExc.has(line.init()))          { keep = false; n.RejInit++;      }
   else if(InitHour.n_elements() > 0 &&
     !InitHour.has(line.init_hour()))   { keep = false; n.RejInitHour++;  }
   else if(Lead.n_elements() > 0 &&
     !Lead.has(line.lead()))            { keep = false; n.RejLead++;      }
   else if(ValidBeg > 0 &&
      line.valid() < ValidBeg)          { keep = false; n.RejValid++;     }
   else if(ValidEnd > 0 &&
      line.valid() > ValidEnd)          { keep = false; n.RejValid++;     }
   else if(ValidInc.n_elements() > 0 &&
     !ValidInc.has(line.valid()))       { keep = false; n.RejValid++;     }
   else if(ValidExc.n_elements() > 0 &&
     ValidExc.has(line.valid()))        { keep = false; n.RejValid++;     }
   else if(ValidHour.n_elements() > 0 &&
     !ValidHour.has(line.valid_hour())) { keep = false; n.RejValidHour++; }
   else if(InitMask.n_elements() > 0 &&
     !InitMask.has(line.init_mask()))   { keep = false; n.RejInitMask++;  }
   else if(ValidMask.n_elements() > 0 &&
     !ValidMask.has(line.valid_mask())) { keep = false; n.RejValidMask++; }
   else if(LineType.n_elements() > 0 &&
     !LineType.has(line.line_type()))   { keep = false; n.RejLineType++;  }

   // Check that PROBRIRW lines include the requested probability type
   else if(line.type() == TCStatLineType_ProbRIRW &&
           !is_bad_data(ProbRIRWThresh) &&
           is_bad_data(get_probrirw_value(line, ProbRIRWThresh))) {
     keep = false;
     n.RejColumnThresh++;
   }

   // Check ColumnThreshMap
   if(keep == true) {

      // Loop through the numeric column thresholds
      for(thr_it=ColumnThreshMap.begin(); thr_it!= ColumnThreshMap.end(); thr_it++) {

         // Get the numeric column value
         v_dbl = get_column_double(line, thr_it->first);

         // Check the column threshold
         if(!thr_it->second.check_dbl(v_dbl)) {
           keep = false;
           n.RejColumnThresh++;
           break;
         }
      }
   }

   // Check ColumnStrMap
   if(keep == true) {

      // Loop through the column string matching
      for(str_it=ColumnStrMap.begin(); str_it!= ColumnStrMap.end(); str_it++) {

         // Retrieve the column value
         v_str = line.get_item(str_it->first.c_str());

         // Check the string value
         if(!str_it->second.has(v_str)) {
           keep = false;
           n.RejColumnStr++;
           break;
         }
      }
   }

   // Retrieve the ADECK and BDECK lat/lon values
   alat = atof(line.get_item("ALAT"));
   alon = atof(line.get_item("ALON"));
   blat = atof(line.get_item("BLAT"));
   blon = atof(line.get_item("BLON"));

   // Check MatchPoints
   if(keep == true && MatchPoints == true) {

      // Check that the ADECK and BDECK locations are both defined
      if(is_bad_data(alat) || is_bad_data(alon) ||
         is_bad_data(blat) || is_bad_data(blon)) {
         keep = false;
         n.RejMatchPoints++;
      }
   }

   // Check the event equalization cases
   if(keep == true && EventEqualSet == true &&
     !EventEqualCases.has(line.header())) {
      keep = false;
      n.RejEventEqual++;
   }

   // Check OutValidMask
   if(keep == true && OutValidMaskName.nonempty()) {

      // Check ADECK locations
      if(is_bad_data(alat) ||
         is_bad_data(alon) ||
         !check_masks(OutValidPolyMask,
                      OutValidGridMask,
                      OutValidAreaMask,
                      alat, alon)) {
         keep = false;
         n.RejOutValidMask++;
      }
   }

   // Update counts
   if(!keep) n.NKeep -= 1;

   return(keep);
}

////////////////////////////////////////////////////////////////////////

double TCStatJob::get_column_double(const TCStatLine &line,
                                    const ConcatString &column) const {
   StringArray sa;
   ConcatString in;
   double v, v_cur;
   bool abs_flag = false;
   int i;

   // Check for PROBRIRW_PROB special case
   if(strcasecmp(column.c_str(), "PROBRIRW_PROB") == 0 &&
      line.type() == TCStatLineType_ProbRIRW) {
      v = get_probrirw_value(line, ProbRIRWThresh);
      return(v);
   }

   // Check for absolute value
   if(strncasecmp(column.c_str(), "ABS", 3) == 0) {
      abs_flag = true;
      sa = column.split("()");
      in = sa[1];
   }
   else {
      in = column;
   }

   // Split the input column name on hyphens for differences
   sa = in.split("-");

   // Get the first value
   v = atof(line.get_item(sa[0].c_str()));

   // If multiple columns, compute the requested difference
   if(sa.n_elements() > 1) {

      // Loop through the column
      for(i=1; i<sa.n_elements(); i++) {

         // Get the current column value
         v_cur = atof(line.get_item(sa[i].c_str()));

         // Compute the difference, checking for bad data
         if(is_bad_data(v) || is_bad_data(v_cur)) v  = bad_data_double;
         else                                     v -= v_cur;
      }
   }

   // Apply absolute value, if requested
   if(abs_flag && !is_bad_data(v)) v = fabs(v);

   return(v);
}

////////////////////////////////////////////////////////////////////////

StringArray TCStatJob::parse_job_command(const char *jobstring) {
   StringArray a, b;
   string c;
   int i;

   // Parse the jobstring into a StringArray
   a.parse_wsss(jobstring);

   // Loop over the StringArray elements
   for(i=0; i<a.n_elements(); i++) {

      c = to_lower(a[i]);

      // Skip newline characters
      if(c.compare("\n") == 0) continue;

      // Check for a job command option
      if(c[0] != '-') {
         b.add(a[i]);
         continue;
      }

      // Check job command options
           if(c.compare("-job"               ) == 0) { JobType = string_to_tcstatjobtype(a[i+1]);         a.shift_down(i, 1); }
      else if(c.compare("-amodel"            ) == 0) { AModel.add_css(a[i+1]);                            a.shift_down(i, 1); }
      else if(c.compare("-bmodel"            ) == 0) { BModel.add_css(a[i+1]);                            a.shift_down(i, 1); }
      else if(c.compare("-desc"              ) == 0) { Desc.add_css(a[i+1]);                              a.shift_down(i, 1); }
      else if(c.compare("-storm_id"          ) == 0) { StormId.add_css(a[i+1]);                           a.shift_down(i, 1); }
      else if(c.compare("-basin"             ) == 0) { Basin.add_css(a[i+1]);                             a.shift_down(i, 1); }
      else if(c.compare("-cyclone"           ) == 0) { Cyclone.add_css(a[i+1]);                           a.shift_down(i, 1); }
      else if(c.compare("-storm_name"        ) == 0) { StormName.add_css(a[i+1]);                         a.shift_down(i, 1); }
      else if(c.compare("-init_beg"          ) == 0) { InitBeg = timestring_to_unix(a[i+1].c_str());      a.shift_down(i, 1); }
      else if(c.compare("-init_end"          ) == 0) { InitEnd = timestring_to_unix(a[i+1].c_str());      a.shift_down(i, 1); }
      else if(c.compare("-init_inc"          ) == 0) { InitInc.add_css(a[i+1].c_str());                   a.shift_down(i, 1); }
      else if(c.compare("-init_exc"          ) == 0) { InitExc.add_css(a[i+1].c_str());                   a.shift_down(i, 1); }
      else if(c.compare("-init_hour"         ) == 0) { InitHour.add_css_sec(a[i+1].c_str());              a.shift_down(i, 1); }
      else if(c.compare("-lead"              ) == 0) { Lead.add_css_sec(a[i+1].c_str());                  a.shift_down(i, 1); }
      else if(c.compare("-lead_req"          ) == 0) { LeadReq.add_css_sec(a[i+1].c_str());               a.shift_down(i, 1); }
      else if(c.compare("-valid_beg"         ) == 0) { ValidBeg = timestring_to_unix(a[i+1].c_str());     a.shift_down(i, 1); }
      else if(c.compare("-valid_end"         ) == 0) { ValidEnd = timestring_to_unix(a[i+1].c_str());     a.shift_down(i, 1); }
      else if(c.compare("-valid_inc"         ) == 0) { ValidInc.add_css(a[i+1].c_str());                  a.shift_down(i, 1); }
      else if(c.compare("-valid_exc"         ) == 0) { ValidExc.add_css(a[i+1].c_str());                  a.shift_down(i, 1); }
      else if(c.compare("-valid_hour"        ) == 0) { ValidHour.add_css_sec(a[i+1].c_str());             a.shift_down(i, 1); }
      else if(c.compare("-init_mask"         ) == 0) { InitMask.add_css(a[i+1].c_str());                  a.shift_down(i, 1); }
      else if(c.compare("-valid_mask"        ) == 0) { ValidMask.add_css(a[i+1].c_str());                 a.shift_down(i, 1); }
      else if(c.compare("-line_type"         ) == 0) { LineType.add_css(a[i+1].c_str());                  a.shift_down(i, 1); }
      else if(c.compare("-track_watch_warn"  ) == 0) { TrackWatchWarn.add_css(a[i+1].c_str());            a.shift_down(i, 1); }
      else if(c.compare("-column_thresh"     ) == 0) { parse_thresh_option(a[i+1].c_str(), a[i+2].c_str(), ColumnThreshMap);
                                                                                                          a.shift_down(i, 2); }
      else if(c.compare("-column_str"        ) == 0) { parse_string_option(a[i+1].c_str(), a[i+2].c_str(), ColumnStrMap);
                                                                                                          a.shift_down(i, 2); }
      else if(c.compare("-init_thresh"       ) == 0) { parse_thresh_option(a[i+1].c_str(), a[i+2].c_str(), InitThreshMap);
                                                                                                          a.shift_down(i, 2); }
      else if(c.compare("-init_str"          ) == 0) { parse_string_option(a[i+1].c_str(), a[i+2].c_str(), InitStrMap);
                                                                                                          a.shift_down(i, 2); }
      else if(c.compare("-water_only"        ) == 0) { WaterOnly = string_to_bool(a[i+1].c_str());        a.shift_down(i, 1); }
      else if(c.compare("-rirw_track"        ) == 0) { RIRWTrack = string_to_tracktype(a[i+1].c_str());   a.shift_down(i, 1); }
      else if(c.compare("-rirw_time"         ) == 0) { RIRWTimeADeck = timestring_to_sec(a[i+1].c_str());
                                                       RIRWTimeBDeck = timestring_to_sec(a[i+1].c_str()); a.shift_down(i, 1); }
      else if(c.compare("-rirw_time_adeck"   ) == 0) { RIRWTimeADeck = timestring_to_sec(a[i+1].c_str()); a.shift_down(i, 1); }
      else if(c.compare("-rirw_time_bdeck"   ) == 0) { RIRWTimeBDeck = timestring_to_sec(a[i+1].c_str()); a.shift_down(i, 1); }
      else if(c.compare("-rirw_exact"        ) == 0) { RIRWExactADeck = string_to_bool(a[i+1].c_str());
                                                       RIRWExactBDeck = string_to_bool(a[i+1].c_str());   a.shift_down(i, 1); }
      else if(c.compare("-rirw_exact_adeck"  ) == 0) { RIRWExactADeck = string_to_bool(a[i+1].c_str());   a.shift_down(i, 1); }
      else if(c.compare("-rirw_exact_bdeck"  ) == 0) { RIRWExactBDeck = string_to_bool(a[i+1].c_str());   a.shift_down(i, 1); }
      else if(c.compare("-rirw_thresh"       ) == 0) { RIRWThreshADeck.set(a[i+1].c_str());
                                                       RIRWThreshBDeck.set(a[i+1].c_str());               a.shift_down(i, 1); }
      else if(c.compare("-rirw_thresh_adeck" ) == 0) { RIRWThreshADeck.set(a[i+1].c_str());               a.shift_down(i, 1); }
      else if(c.compare("-rirw_thresh_bdeck" ) == 0) { RIRWThreshBDeck.set(a[i+1].c_str());               a.shift_down(i, 1); }
      else if(c.compare("-rirw_window"       ) == 0) {
         if(i+2 < a.n_elements() && is_number(a[i+2].c_str())) {
                                                       RIRWWindowBeg = timestring_to_sec(a[i+1].c_str());
                                                       RIRWWindowEnd = timestring_to_sec(a[i+2].c_str()); a.shift_down(i, 2); }
         else {
                                                       RIRWWindowEnd = timestring_to_sec(a[i+1].c_str());
                                                       RIRWWindowBeg = -1 * RIRWWindowEnd;                a.shift_down(i, 1); }
      }
      else if(c.compare("-probrirw_thresh"   ) == 0) { ProbRIRWThresh = atof(a[i+1].c_str());             a.shift_down(i, 1); }
      else if(c.compare("-landfall"          ) == 0) { Landfall = string_to_bool(a[i+1].c_str());         a.shift_down(i, 1); }
      else if(c.compare("-landfall_window")    == 0) {
                                                       Landfall = true; // For -landfall_window, set -landfall true
         if(i+2 < a.n_elements() && is_number(a[i+2].c_str())) {
                                                       LandfallBeg = timestring_to_sec(a[i+1].c_str());
                                                       LandfallEnd = timestring_to_sec(a[i+2].c_str());   a.shift_down(i, 2); }
         else {
                                                       LandfallEnd = timestring_to_sec(a[i+1].c_str());
                                                       LandfallBeg = -1 * LandfallEnd;                    a.shift_down(i, 1); }
      }
      else if(c.compare("-match_points"      ) == 0) { MatchPoints = string_to_bool(a[i+1].c_str());      a.shift_down(i, 1); }
      else if(c.compare("-event_equal"       ) == 0) { EventEqual = string_to_bool(a[i+1].c_str());       a.shift_down(i, 1); }
      else if(c.compare("-event_equal_lead"  ) == 0) { EventEqualLead.add_css_sec(a[i+1].c_str());        a.shift_down(i, 1); }
      else if(c.compare("-out_init_mask"     ) == 0) { set_out_init_mask(a[i+1].c_str());                 a.shift_down(i, 1); }
      else if(c.compare("-out_valid_mask"    ) == 0) { set_out_valid_mask(a[i+1].c_str());                a.shift_down(i, 1); }
      else if(c.compare("-dump_row"          ) == 0) { DumpFile = a[i+1]; open_dump_file();               a.shift_down(i, 1); }
      else if(c.compare("-out_stat"          ) == 0) { StatFile = a[i+1]; open_stat_file();               a.shift_down(i, 1); }
      else                                           {                                                    b.add(a[i]);        }
   }

   return(b);
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::set_out_init_mask(const char *poly_file) {

   if(poly_file) {
      mlog << Debug(2)
           << "Init Points Masking File: " << poly_file << "\n";
      OutInitMaskFile = poly_file;
      parse_poly_mask(OutInitMaskFile, OutInitPolyMask,
                      OutInitGridMask, OutInitAreaMask,
                      OutInitMaskName);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::set_out_valid_mask(const char *poly_file) {

   if(poly_file) {
      mlog << Debug(2)
           << "Valid Points Masking File: " << poly_file << "\n";
      OutValidMaskFile = poly_file;
      parse_poly_mask(OutValidMaskFile, OutValidPolyMask,
                      OutValidGridMask, OutValidAreaMask,
                      OutValidMaskName);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::open_dump_file() {

   close_dump_file();

   if(DumpFile.empty()) return;

   DumpOut = new ofstream;
   met_open(*DumpOut, DumpFile.c_str());

   if(!DumpOut) {
      mlog << Error << "\nTCStatJob::open_dump_file()-> "
           << "can't open the output file \"" << DumpFile
           << "\" for writing!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::close_dump_file() {

   if(DumpOut) {

      mlog << Debug(1)
           << "Creating output dump file: " << DumpFile << "\n";

      DumpOut->close();
      delete DumpOut;
      DumpOut = (ofstream *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::open_stat_file() {

   close_stat_file();

   if(StatFile.empty()) return;

   StatOut = new ofstream;
   met_open(*StatOut, StatFile.c_str());

   if(!StatOut) {
      mlog << Error << "\nTCStatJob::open_stat_file()-> "
           << "can't open the output file \"" << StatFile
           << "\" for writing!\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::close_stat_file() {

   if(StatOut) {

      mlog << Debug(1)
           << "Creating output statistics file: " << StatFile << "\n";

      StatOut->close();
      delete StatOut;
      StatOut = (ofstream *) 0;
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::dump_pair(const TrackPairInfo &pair, ofstream *out) {

   if(!out || pair.n_points() == 0) return;

   TcHdrColumns tchc;
   AsciiTable out_at;
   int i_row, hdr_row;

   // Determine if we need to write a header row
   if((int) out->tellp() == 0) hdr_row = 1;
   else                        hdr_row = 0;

   // Initialize the output AsciiTable
   out_at.set_size(pair.n_points() + hdr_row,
                   n_tc_header_cols + n_tc_mpr_cols);

   // Write the TCMPR header row
   write_tc_mpr_header_row(1, out_at, 0, 0);

   // Setup the output AsciiTable
   justify_tc_stat_cols(out_at);
   out_at.set_precision(get_precision());
   out_at.set_bad_data_value(bad_data_double);
   out_at.set_bad_data_str(na_str);
   out_at.set_delete_trailing_blank_rows(1);

   // Setup header columns
   tchc.clear();
   tchc.set_adeck_model(pair.adeck().technique());
   tchc.set_bdeck_model(pair.bdeck().technique());
   tchc.set_storm_id(pair.adeck().storm_id());
   tchc.set_basin(pair.bdeck().basin());
   tchc.set_cyclone(pair.bdeck().cyclone());
   tchc.set_storm_name(pair.bdeck().storm_name());

   if(OutInitMaskName.nonempty())  tchc.set_init_mask(OutInitMaskName);
   else                            tchc.set_init_mask(na_string);
   if(OutValidMaskName.nonempty()) tchc.set_valid_mask(OutValidMaskName);
   else                            tchc.set_valid_mask(na_string);

   // Write the TrackPairInfo object
   i_row = hdr_row;
   write_tc_mpr_row(tchc, pair, out_at, i_row);

   // Write the AsciiTable to the file
   *out << out_at;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::dump_line(const TCStatLine &line, ofstream *out) {

   if(!out) return;

   *out << line;

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString TCStatJob::serialize() const {
   ConcatString s;
   int i;
   map<ConcatString,ThreshArray>::const_iterator thr_it;
   map<ConcatString,StringArray>::const_iterator str_it;

   // Initialize the jobstring
   s.clear();
   s.set_precision(get_precision());

   if(JobType != NoTCStatJobType)
      s << "-job " << tcstatjobtype_to_string(JobType) << " ";
   for(i=0; i<AModel.n_elements(); i++)
      s << "-amodel " << AModel[i] << " ";
   for(i=0; i<BModel.n_elements(); i++)
      s << "-bmodel " << BModel[i] << " ";
   for(i=0; i<Desc.n_elements(); i++)
      s << "-desc " << Desc[i] << " ";
   for(i=0; i<StormId.n_elements(); i++)
      s << "-storm_id " << StormId[i] << " ";
   for(i=0; i<Basin.n_elements(); i++)
      s << "-basin " << Basin[i] << " ";
   for(i=0; i<Cyclone.n_elements(); i++)
      s << "-cyclone " << Cyclone[i] << " ";
   for(i=0; i<StormName.n_elements(); i++)
      s << "-storm_name " << StormName[i] << " ";
   if(InitBeg > 0)
      s << "-init_beg " << unix_to_yyyymmdd_hhmmss(InitBeg) << " ";
   if(InitEnd > 0)
      s << "-init_end " << unix_to_yyyymmdd_hhmmss(InitEnd) << " ";
   for(i=0; i<InitInc.n_elements(); i++)
      s << "-init_inc " << unix_to_yyyymmdd_hhmmss(InitInc[i]) << " ";
   for(i=0; i<InitExc.n_elements(); i++)
      s << "-init_exc " << unix_to_yyyymmdd_hhmmss(InitExc[i]) << " ";
   for(i=0; i<InitHour.n_elements(); i++)
      s << "-init_hour " << sec_to_hhmmss(nint(InitHour[i])) << " ";
   for(i=0; i<Lead.n_elements(); i++)
      s << "-lead " << sec_to_hhmmss(nint(Lead[i])) << " ";
   for(i=0; i<LeadReq.n_elements(); i++)
         s << "-lead_req " << sec_to_hhmmss(nint(LeadReq[i])) << " ";
   if(ValidBeg > 0)
      s << "-valid_beg " << unix_to_yyyymmdd_hhmmss(ValidBeg) << " ";
   if(ValidEnd > 0)
      s << "-valid_end " << unix_to_yyyymmdd_hhmmss(ValidEnd) << " ";
   for(i=0; i<ValidInc.n_elements(); i++)
      s << "-valid_inc " << unix_to_yyyymmdd_hhmmss(ValidInc[i]) << " ";
   for(i=0; i<ValidExc.n_elements(); i++)
      s << "-valid_exc " << unix_to_yyyymmdd_hhmmss(ValidExc[i]) << " ";
   for(i=0; i<ValidHour.n_elements(); i++)
      s << "-valid_hour " << sec_to_hhmmss(nint(ValidHour[i])) << " ";
   for(i=0; i<InitMask.n_elements(); i++)
      s << "-init_mask " << InitMask[i] << " ";
   for(i=0; i<ValidMask.n_elements(); i++)
      s << "-valid_mask " << ValidMask[i] << " ";
   for(i=0; i<LineType.n_elements(); i++)
      s << "-line_type " << LineType[i] << " ";
   for(i=0; i<TrackWatchWarn.n_elements(); i++)
      s << "-track_watch_warn " << TrackWatchWarn[i] << " ";
   for(thr_it=ColumnThreshMap.begin(); thr_it!= ColumnThreshMap.end(); thr_it++) {
      for(i=0; i<thr_it->second.n_elements(); i++) {
         s << "-column_thresh " << thr_it->first << " "
           << thr_it->second[i].get_str() << " ";
      }
   }
   for(str_it=ColumnStrMap.begin(); str_it!= ColumnStrMap.end(); str_it++) {
      for(i=0; i<str_it->second.n_elements(); i++) {
         s << "-column_str " << str_it->first << " "
           << str_it->second[i] << " ";
      }
   }
   for(thr_it=InitThreshMap.begin(); thr_it!= InitThreshMap.end(); thr_it++) {
      for(i=0; i<thr_it->second.n_elements(); i++) {
         s << "-init_thresh " << thr_it->first << " "
           << thr_it->second[i].get_str() << " ";
      }
   }
   for(str_it=InitStrMap.begin(); str_it!= InitStrMap.end(); str_it++) {
      for(i=0; i<str_it->second.n_elements(); i++) {
         s << "-init_str " << str_it->first << " "
           << str_it->second[i] << " ";
      }
   }
   if(WaterOnly != default_water_only)
      s << "-water_only " << bool_to_string(WaterOnly) << " ";
   if(RIRWTrack != default_rirw_track) {
      s << "-rirw_track " << tracktype_to_string(RIRWTrack) << " ";
      if(RIRWTimeADeck == RIRWTimeBDeck) {
         s << "-rirw_time " << sec_to_hhmmss(RIRWTimeADeck) << " ";
      }
      else {
         s << "-rirw_time_adeck " << sec_to_hhmmss(RIRWTimeADeck) << " "
           << "-rirw_time_bdeck " << sec_to_hhmmss(RIRWTimeBDeck) << " ";
      }
      if(RIRWExactADeck == RIRWExactBDeck) {
         s << "-rirw_exact " << bool_to_string(RIRWExactADeck) << " ";
      }
      else {
         s << "-rirw_exact_adeck " << bool_to_string(RIRWExactADeck) << " "
           << "-rirw_exact_bdeck " << bool_to_string(RIRWExactBDeck) << " ";
      }
      if(RIRWThreshADeck == RIRWThreshBDeck) {
         s << "-rirw_thresh " << RIRWThreshADeck.get_str() << " ";
      }
      else {
         s << "-rirw_thresh_adeck " << RIRWThreshADeck.get_str() << " "
           << "-rirw_thresh_bdeck " << RIRWThreshBDeck.get_str() << " ";
      }
      s << "-rirw_window " << sec_to_hhmmss(RIRWWindowBeg) << " "
        << sec_to_hhmmss(RIRWWindowEnd) << " ";
   }
   if(ProbRIRWThresh != default_probrirw_thresh) {
      s << "-probrirw_thresh " << ProbRIRWThresh << " ";
   }
   if(Landfall != default_landfall) {
      s << "-landfall " << bool_to_string(Landfall) << " "
        << "-landfall_window " << sec_to_hhmmss(LandfallBeg) << " "
        << sec_to_hhmmss(LandfallEnd) << " ";
   }
   if(MatchPoints != default_match_points)
      s << "-match_points " << bool_to_string(MatchPoints) << " ";
   if(EventEqual != default_event_equal)
      s << "-event_equal " << bool_to_string(EventEqual) << " ";
   for(i=0; i<EventEqualLead.n_elements(); i++)
      s << "-event_equal_lead " << sec_to_hhmmss(nint(EventEqualLead[i])) << " ";
   if(OutInitMaskFile.nonempty())
      s << "-out_init_mask " << OutInitMaskFile << " ";
   if(OutValidMaskFile.nonempty())
      s << "-out_valid_mask " << OutValidMaskFile << " ";
   if(DumpFile.length() > 0)
      s << "-dump_row " << DumpFile << " ";
   if(StatFile.length() > 0)
      s << "-stat_row " << StatFile << " ";

   return(s);
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::do_job(const StringArray &file_list, TCLineCounts &n) {

   mlog << Error << "\nTCStatJob::do_job() -> "
        << "the do_job() base class function should never be called!\n\n";
   exit(1);

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Event equalize track data to build list of common cases.
//
////////////////////////////////////////////////////////////////////////

void TCStatJob::event_equalize_tracks() {
   TrackPairInfo pair;
   TCLineCounts n;
   ConcatString key, val;
   StringArray case_list;
   ConcatString models;
   bool skip;
   int i;

   // Event equalization case map
   map<ConcatString,StringArray,cs_cmp> case_map;
   map<ConcatString,StringArray,cs_cmp>::iterator it;

   mlog << Debug(3)
        << "Applying track-based event equalization logic.\n";

   // Rewind to the beginning of the track pair input
   TCSTFiles.rewind();

   // Process each of the track pairs
   while(TCSTFiles >> pair) {

      // Subset the track pair down to points to be used
      subset_track_pair(pair, n);

      // Skip tracks missing any of the required lead times
      for(i=0, skip=false; i<EventEqualLead.n_elements(); i++) {
         if(pair.adeck().lead_index(EventEqualLead[i]) < 0) {
            skip = true;
            break;
         }
      }
      if(skip) continue;

      // Add event equalization information for each track point
      for(i=0; i<pair.n_lines(); i++) {

         // Store the current map key and value
         key = pair.adeck().technique(); // ADECK model name
         val = pair.line(i)->header();   // Track line header

         // Add a new map entry, if necessary
         if(case_map.count(key) == 0) {
            case_list.clear();
            case_map[key] = case_list;
         }

         // Add the current header to the case map
         if(!case_map[key].has(val)) case_map[key].add(val);

      } // end for i
   } // end while

   // Initialize to the first map entry
   EventEqualCases.clear();
   if(case_map.size() > 0) EventEqualCases = case_map.begin()->second;

   // Loop over the map entries and build a list of common cases
   for(it=case_map.begin(); it!= case_map.end(); it++) {
      EventEqualCases = intersection(EventEqualCases, it->second);
      models << it->first << " ";
   } // end for it

   mlog << Debug(3)
        << "For track-based event equalization, identified "
        << EventEqualCases.n_elements() << " common cases for "
        << (int) case_map.size() << " models: " << models << "\n";

   for(i=0; i<EventEqualCases.n_elements(); i++) {
      mlog << Debug(4)
           << "[Case " << i+1 << " of " << EventEqualCases.n_elements()
           << "] " << EventEqualCases[i] << "\n";
   }

   // Set flag to indicate that event equalization has been applied.
   EventEqualSet = true;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Event equalize TCStatLine data to build list of common cases.
//
////////////////////////////////////////////////////////////////////////

void TCStatJob::event_equalize_lines() {
   TCLineCounts n;
   TCStatLine line;
   ConcatString key, val;
   StringArray case_list;
   ConcatString models;
   int i;

   // Event equalization case map
   map<ConcatString,StringArray,cs_cmp> case_map;
   map<ConcatString,StringArray,cs_cmp>::iterator it;

   mlog << Debug(3)
        << "Applying line-based event equalization logic.\n";

   // Rewind to the beginning of the track pair input
   TCSTFiles.rewind();

   // Process each of the lines
   while(TCSTFiles >> line) {

      // Check if this line should be kept
      if(!is_keeper_line(line, n)) continue;

      // Store the current map key and value
      key = line.amodel(); // ADECK model name
      val = line.header(); // Track line header

      // Add a new map entry, if necessary
      if(case_map.count(key) == 0) {
         case_list.clear();
         case_map[key] = case_list;
      }

      // Add the current header to the case map
      if(!case_map[key].has(val)) case_map[key].add(val);

   } // end while

   // Initialize to the first map entry
   EventEqualCases.clear();
   if(case_map.size() > 0) EventEqualCases = case_map.begin()->second;

   // Loop over the map entries and build a list of common cases
   for(it=case_map.begin(); it!= case_map.end(); it++) {
      EventEqualCases = intersection(EventEqualCases, it->second);
      models << it->first << " ";
   } // end for it

   mlog << Debug(3)
        << "For line-based event equalization, identified "
        << EventEqualCases.n_elements() << " common cases for "
        << (int) case_map.size() << " models: " << models << "\n";

   for(i=0; i<EventEqualCases.n_elements(); i++) {
      mlog << Debug(4)
           << "[Case " << i+1 << " of " << EventEqualCases.n_elements()
           << "] " << EventEqualCases[i] << "\n";
   }

   // Set flag to indicate that event equalization has been applied.
   EventEqualSet = true;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::subset_track_pair(TrackPairInfo &pair, TCLineCounts &n) {
   int i, n_rej;

   // Check for no points
   if(pair.n_points() == 0) return;

   // Increment the read and keep counts
   n.NRead += pair.n_points();
   n.NKeep += pair.n_points();

   // Determine if the whole track can be discarded
   if(!is_keeper_track(pair, n)) {
      pair.clear();
      return;
   }

   // Check WaterOnly
   if(WaterOnly == true) {

      // Determine the rapid intensification points
      n_rej = pair.check_water_only();

      // Update counts
      n.RejWaterOnly += n_rej;
      n.NKeep        -= n_rej;
   }

   // Check RIRW
   if(RIRWTrack != TrackType_None) {

      // Determine the rapid intensification points
      n_rej = pair.check_rirw(RIRWTrack,
                              RIRWTimeADeck, RIRWTimeBDeck,
                              RIRWExactADeck, RIRWExactBDeck,
                              RIRWThreshADeck, RIRWThreshBDeck);

      // Update counts
      n.RejRIRW += n_rej;
      n.NKeep   -= n_rej;
   }

   // Check Landfall
   if(Landfall == true) {

      // Determine the landfall points
      n_rej = pair.check_landfall(LandfallBeg, LandfallEnd);

      // Update counts
      n.RejLandfall += n_rej;
      n.NKeep       -= n_rej;
   }

   // Loop over the track points to check line by line
   for(i=0; i<pair.n_lines(); i++) {

      // Skip marked lines
      if(!pair.keep(i)) continue;

      // Check if this line should be discarded
      if(!is_keeper_line(*pair.line(i), n)) pair.set_keep(i, 0);
   }

   // Subset the points marked for retention
   pair = pair.keep_subset();

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class TCStatJobFilter
//
////////////////////////////////////////////////////////////////////////

TCStatJobFilter::TCStatJobFilter() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCStatJobFilter::~TCStatJobFilter() {

   clear();
}

////////////////////////////////////////////////////////////////////////

TCStatJobFilter::TCStatJobFilter(const TCStatJobFilter &j) {

   init_from_scratch();

   assign(j);
}

////////////////////////////////////////////////////////////////////////

TCStatJobFilter & TCStatJobFilter::operator=(const TCStatJobFilter &j) {

   if(this == &j) return(*this);

   assign(j);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobFilter::init_from_scratch() {

   TCStatJob::init_from_scratch();

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobFilter::clear() {

   TCStatJob::clear();

   JobType = TCStatJobType_Filter;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobFilter::assign(const TCStatJobFilter & j) {

   TCStatJob::assign(j);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobFilter::do_job(const StringArray &file_list,
                             TCLineCounts &n) {
   int i;

   // Check that the -dump_row option has been supplied
   if(!DumpOut) {
      mlog << Error << "\nTCStatJobFilter::do_job() -> "
           << "this function may only be called when using the "
           << "-dump_row option in the job command line:\n"
           << serialize() << "\n\n";
      exit(1);
   }

   //
   // Apply logic based on the LineType
   //

   // If not specified, assume TCMPR by adding it to the LineType
   if(LineType.n_elements() == 0) LineType.add(TCStatLineType_TCMPR_Str);

   // Add the input file list
   TCSTFiles.add_files(file_list);

   // Process track-based filter job
   if(LineType.has(TCStatLineType_TCMPR_Str)) {

      // TCMPR and non-TCMPR LineTypes cannot be mixed
      for(i=0; i<LineType.n_elements(); i++) {
         if(strcasecmp(LineType[i].c_str(), TCStatLineType_TCMPR_Str) != 0) {
            mlog << Error << "\nTCStatJobFilter::do_job() -> "
                 << "the track-based " << TCStatLineType_TCMPR_Str
                 << " line type cannot be mixed with the "
                 << LineType[i] << " line type.\n\n";
            exit(1);
         }
      } // end for

      filter_tracks(n);
   }

   // Process line-based filter job
   else {
      filter_lines(n);
   }

   // Close the dump file
   if(DumpOut) close_dump_file();

   // Process the filter output
   if(JobOut) do_output(*JobOut);
   else       do_output(cout);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobFilter::filter_tracks(TCLineCounts &n) {
   TrackPairInfo pair;

   // Apply the event equalization logic to build a list of common cases
   if(EventEqual == true) event_equalize_tracks();

   // Check for no common cases
   if(EventEqualSet == true && EventEqualCases.n_elements() == 0) {
      mlog << Debug(1)
           << "Event equalization of tracks found no common cases.\n";
   }
   // Otherwise, process the track data
   else {

      // Rewind to the beginning of the input
      TCSTFiles.rewind();

      // Process each of the track pairs
      while(TCSTFiles >> pair) {

         // Process the track pair down to points to be used
         subset_track_pair(pair, n);

         // Write out the retained lines
         if(pair.n_points() > 0) {

            mlog << Debug(4)
                 << "Processing track pair: " << pair.case_info() << "\n";

            if(DumpOut) dump_pair(pair, DumpOut);
         }
      } // end while
   } // end else

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobFilter::filter_lines(TCLineCounts &n) {
   TCStatLine line;

   // Apply the event equalization logic to build a list of common cases
   if(EventEqual == true) event_equalize_lines();

   // Check for no common cases
   if(EventEqualSet == true && EventEqualCases.n_elements() == 0) {
      mlog << Debug(1)
           << "Event equalization of lines found no common cases.\n";
   }
   // Otherwise, process the line data
   else {

      // Rewind to the beginning of the input
      TCSTFiles.rewind();

      // Process each of the pair lines
      while(TCSTFiles >> line) {

         // Increment the read and keep counts
         n.NRead++;
         n.NKeep++;

         // Check if this line should be kept
         if(!is_keeper_line(line, n)) continue;

         if(DumpOut) dump_line(line, DumpOut);

      } // end while
   } // end else

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobFilter::do_output(ostream &out) {
   ConcatString line;

   // Build a simple output line
   line << "FILTER: " << serialize() << "\n";
   out  << line << "\n";

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class TCStatJobSummary
//
////////////////////////////////////////////////////////////////////////

TCStatJobSummary::TCStatJobSummary() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCStatJobSummary::~TCStatJobSummary() {

   clear();
}

////////////////////////////////////////////////////////////////////////

TCStatJobSummary::TCStatJobSummary(const TCStatJobSummary &j) {

   init_from_scratch();

   assign(j);
}

////////////////////////////////////////////////////////////////////////

TCStatJobSummary & TCStatJobSummary::operator=(const TCStatJobSummary &j) {

   if(this == &j) return(*this);

   assign(j);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::init_from_scratch() {

   TCStatJob::init_from_scratch();

   // Ignore case when performing comparisons
   ReqColumn.set_ignore_case(1);
   Column.set_ignore_case(1);
   ByColumn.set_ignore_case(1);

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::clear() {

   TCStatJob::clear();

   JobType = TCStatJobType_Summary;

   ReqColumn.clear();
   Column.clear();
   ByColumn.clear();
   SummaryMap.clear();

   // Set to default value
   ColumnUnion = default_column_union;
   OutAlpha    = default_tc_alpha;
   FSPThresh   = default_fsp_thresh;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::assign(const TCStatJobSummary & j) {

   TCStatJob::assign(j);

   ReqColumn   = j.ReqColumn;
   Column      = j.Column;
   ColumnUnion = j.ColumnUnion;
   ByColumn    = j.ByColumn;
   SummaryMap  = j.SummaryMap;
   OutAlpha    = j.OutAlpha;
   FSPThresh   = j.FSPThresh;

   return;
}

////////////////////////////////////////////////////////////////////////

StringArray TCStatJobSummary::parse_job_command(const char *jobstring) {
   StringArray a, b;
   string c;
   int i;

   // Call the parent and store any unused options
   a = TCStatJob::parse_job_command(jobstring);

   // Loop over the StringArray elements
   for(i=0; i<a.n_elements(); i++) {

      // Point at the current entry
      c = to_lower(a[i]);

      // Check for a job command option
      if(c[0] != '-') {
         b.add(a[i]);
         continue;
      }

      // Check job command options
           if(c.compare("-column"      ) == 0) { ReqColumn.add_css(to_upper(a[i+1]));
                                                 add_column(a[i+1].c_str());                   a.shift_down(i, 1); }
      else if(c.compare("-column_union") == 0) { ColumnUnion = string_to_bool(a[i+1].c_str()); a.shift_down(i, 1); }
      else if(c.compare("-by"          ) == 0) { ByColumn.add_css(to_upper(a[i+1]));           a.shift_down(i, 1); }
      else if(c.compare("-out_alpha"   ) == 0) { OutAlpha = atof(a[i+1].c_str());              a.shift_down(i, 1); }
      else if(c.compare("-fsp_thresh"  ) == 0) { FSPThresh.set(a[i+1].c_str());                a.shift_down(i, 1); }
      else                                     {                                               b.add(a[i]);        }
   }

   return(b);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::add_column(const char *c) {
   int i, j;
   StringArray sa;

   // Parse entries into a StringArray
   sa.add_css(c);

   // Loop over the entries, handling special column names
   for(i=0; i<sa.n_elements(); i++) {

      // Track errors
      if(strcasecmp(sa[i].c_str(), "TRACK") == 0) {
         for(j=0; j<n_tc_cols_track; j++) Column.add(tc_cols_track[j]);
      }
      // Wind errors
      else if(strcasecmp(sa[i].c_str(), "WIND") == 0) {
         for(j=0; j<n_tc_cols_wind; j++) Column.add(tc_cols_wind[j]);
      }
      // Track and Intensity (TI)
      else if(strcasecmp(sa[i].c_str(), "TI") == 0) {
         for(j=0; j<n_tc_cols_ti; j++) Column.add(tc_cols_ti[j]);
      }
      // Along and Cross Track (AC)
      else if(strcasecmp(sa[i].c_str(), "AC") == 0) {
         for(j=0; j<n_tc_cols_ac; j++) Column.add(tc_cols_ac[j]);
      }
      // Lat/Lon Difference (XY)
      else if(strcasecmp(sa[i].c_str(), "XY") == 0) {
         for(j=0; j<n_tc_cols_xy; j++) Column.add(tc_cols_xy[j]);
      }
      // Otherwise, just add the column name
      else {
         Column.add(sa[i]);
      }
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString TCStatJobSummary::serialize() const {
   ConcatString s;
   int i;

   // Call the parent
   s = TCStatJob::serialize();

   // Add summary job-specific options
   for(i=0; i<ReqColumn.n_elements(); i++)
      s << "-column " << ReqColumn[i] << " ";
   if(ColumnUnion != default_column_union)
      s << "-column_union " << bool_to_string(ColumnUnion) << " ";
   for(i=0; i<ByColumn.n_elements(); i++)
      s << "-by " << ByColumn[i] << " ";
   if(!(FSPThresh == default_fsp_thresh))
      s << "-fsp_thresh " << FSPThresh.get_str();

   // Always list the output alpha value used
   s << "-out_alpha " << OutAlpha << " ";

   return(s);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::do_job(const StringArray &file_list,
                              TCLineCounts &n) {
   TrackPairInfo pair;
   int i;

   // Check that the -column option has been supplied
   if(Column.n_elements() == 0) {
      mlog << Error << "\nTCStatJobSummary::do_job() -> "
           << "this function may only be called when using the "
           << "-column option in the job command line:\n"
           << serialize() << "\n\n";
      exit(1);
   }

   //
   // Apply logic based on the LineType
   //

   // If not specified, assume TCMPR by adding it to the LineType
   if(LineType.n_elements() == 0) LineType.add(TCStatLineType_TCMPR_Str);

   // Add the input file list
   TCSTFiles.add_files(file_list);

   // Process track-based filter job
   if(LineType.has(TCStatLineType_TCMPR_Str)) {

      // TCMPR and non-TCMPR LineTypes cannot be mixed
      for(i=0; i<LineType.n_elements(); i++) {
         if(strcasecmp(LineType[i].c_str(), TCStatLineType_TCMPR_Str) != 0) {
            mlog << Error << "\nTCStatJobSummary::do_job() -> "
                 << "the track-based " << TCStatLineType_TCMPR_Str
                 << " line type cannot be mixed with the "
                 << LineType[i] << " line type.\n\n";
            exit(1);
         }
      } // end for

      summarize_tracks(n);
   }

   // Process line-based summary job
   else {
      summarize_lines(n);
   }

   // Close the dump file
   if(DumpOut) close_dump_file();

   // Process the filter output
   if(JobOut) do_output(*JobOut);
   else       do_output(cout);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::summarize_tracks(TCLineCounts &n) {
   TrackPairInfo pair;

   // Apply the event equalization logic to build a list of common cases
   if(EventEqual == true) event_equalize_tracks();

   // Check for no common cases
   if(EventEqualSet == true && EventEqualCases.n_elements() == 0) {
      mlog << Debug(1)
           << "Event equalization of tracks found no common cases.\n";
   }
   // Otherwise, process the track data
   else {

      // Rewind to the beginning of the track pair input
      TCSTFiles.rewind();

      // Process each of the track pairs
      while(TCSTFiles >> pair) {

         // Process the track pair down to points to be used
         subset_track_pair(pair, n);

         // Write out the retained lines
         if(pair.n_points() > 0) {

            mlog << Debug(4)
                 << "Processing pair: " << pair.case_info() << "\n";

            // Process the track pair info for the summary job
            process_pair(pair);

            if(DumpOut) dump_pair(pair, DumpOut);
         }
      } // end while
   } // end else

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::summarize_lines(TCLineCounts &n) {
   TCStatLine line;

   // Apply the event equalization logic to build a list of common cases
   if(EventEqual == true) event_equalize_lines();

   // Check for no common cases
   if(EventEqualSet == true && EventEqualCases.n_elements() == 0) {
      mlog << Debug(1)
           << "Event equalization of lines found no common cases.\n";
   }
   // Otherwise, process the line data
   else {

      // Rewind to the beginning of the track pair input
      TCSTFiles.rewind();

      // Process each of the track pairs
      while(TCSTFiles >> line) {

         // Increment the read and keep counts
         n.NRead++;
         n.NKeep++;

         // Check if this line should be kept
         if(!is_keeper_line(line, n)) continue;

         // Process the line for the summary job
         process_line(line);

         if(DumpOut) dump_line(line, DumpOut);

      } // end while
   } // end else

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::process_pair(TrackPairInfo &pair) {
   int i, j;
   map<ConcatString,SummaryMapData,cs_cmp> cur_map;
   ConcatString prefix, key, cur;
   SummaryMapData data;
   double val;

   // Initialize the map
   cur_map.clear();

   // Loop over TCStatLines and construct a summary map
   for(i=0; i<pair.n_lines(); i++) {

      // Add summary info to the current map
      for(j=0; j<Column.n_elements(); j++) {

         // Build the key and get the current column value
         // For -column_union, put all columns in the same key
         if(ColumnUnion) prefix = write_css(ReqColumn);
         else            prefix = Column[j];
         key = build_map_key(prefix.c_str(), *pair.line(i), ByColumn);
         val = get_column_double(*pair.line(i), Column[j]);

         // Add map entry for this key, if necessary
         if(cur_map.count(key) == 0) cur_map[key] = data;

         // Add values for this key
         cur_map[key].Val.add(val);
         cur_map[key].Hdr.add(pair.line(i)->header());
         cur_map[key].AModel.add(pair.line(i)->amodel());
         cur_map[key].Init.add(pair.line(i)->init());
         cur_map[key].Lead.add(pair.line(i)->lead());
         cur_map[key].Valid.add(pair.line(i)->valid());

      } // end for j
   } // end for i

   // Add the current map
   add_map(cur_map);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::process_line(TCStatLine &line) {
   int i;
   map<ConcatString,SummaryMapData,cs_cmp> cur_map;
   ConcatString prefix, key, cur;
   SummaryMapData data;
   double val;

   // Initialize the map
   cur_map.clear();

   // Add summary info to the current map
   for(i=0; i<Column.n_elements(); i++) {

      // Build the key and get the current column value
      // For -column_union, put all columns in the same key
      if(ColumnUnion) prefix = write_css(Column);
      else            prefix = Column[i];
      key = build_map_key(prefix.c_str(), line, ByColumn);
      val = get_column_double(line, Column[i]);

      // Add map entry for this key, if necessary
      if(cur_map.count(key) == 0) cur_map[key] = data;

      // Add values for this key
      cur_map[key].Val.add(val);
      cur_map[key].Hdr.add(line.header());
      cur_map[key].AModel.add(line.amodel());
      cur_map[key].Init.add(line.init());
      cur_map[key].Lead.add(line.lead());
      cur_map[key].Valid.add(line.valid());
   } // end for i

   // Add the current map
   add_map(cur_map);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::add_map(map<ConcatString,SummaryMapData,cs_cmp>&m) {
   map<ConcatString,SummaryMapData,cs_cmp>::iterator it;

   // Loop over the input map entries
   for(it=m.begin(); it!= m.end(); it++) {

      // Current map key is not yet defined in SummaryMap
      if(SummaryMap.count(it->first) == 0) {

         mlog << Debug(5)
              << "Summary Map Insert (" << it->first << ") "
              << it->second.Val.n_elements() << " values: "
              << it->second.Val.serialize() << "\n";

         // Add the pair to the map
         SummaryMap[it->first] = it->second;
      }
      // Current map key is defined in SummaryMap
      else {

         mlog << Debug(5)
              << "Summary Map Add (" << it->first << ") "
              << it->second.Val.n_elements() << " values: "
              << it->second.Val.serialize() << "\n";

         // Add the value for the existing key
         SummaryMap[it->first].Val.add(it->second.Val);
         SummaryMap[it->first].Hdr.add(it->second.Hdr);
         SummaryMap[it->first].AModel.add(it->second.AModel);
         SummaryMap[it->first].Init.add(it->second.Init);
         SummaryMap[it->first].Lead.add(it->second.Lead);
         SummaryMap[it->first].Valid.add(it->second.Valid);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::do_output(ostream &out) {
   map<ConcatString,SummaryMapData,cs_cmp>::iterator it;
   StringArray sa;
   ConcatString line;
   AsciiTable out_at;
   NumArray v, lead, index;
   TimeArray init, valid;
   CIInfo mean_ci, stdev_ci;
   int i, r, c, dsec, tind;

   double fsp;
   NumArray fsp_total, fsp_best, fsp_ties;

   // Setup the output table
   out_at.set_size((int) SummaryMap.size() + 1,
                   ByColumn.n_elements() + 24);

   // Left-justify case info and right-justify summary output
   for(i=0; i<out_at.ncols(); i++) {
      if(i < ByColumn.n_elements()) out_at.set_column_just(i, LeftJust);
      else                            out_at.set_column_just(i, RightJust);
   }
   out_at.set_precision(get_precision());
   out_at.set_bad_data_value(bad_data_double);
   out_at.set_bad_data_str(na_str);
   out_at.set_delete_trailing_blank_rows(1);

   // Set up CIInfo objects
   mean_ci.allocate_n_alpha(1);
   stdev_ci.allocate_n_alpha(1);

   // Initialize row and column indices
   r = c = 0;

   // Write the header row
   out_at.set_entry(r, c++, "COL_NAME:");
   out_at.set_entry(r, c++, "COLUMN");

   // Write case column names
   for(i=0; i<ByColumn.n_elements(); i++) {
      out_at.set_entry(r, c++, ByColumn[i]);
   }

   out_at.set_entry(r, c++, "TOTAL");
   out_at.set_entry(r, c++, "VALID");
   out_at.set_entry(r, c++, "MEAN");
   out_at.set_entry(r, c++, "MEAN_NCL");
   out_at.set_entry(r, c++, "MEAN_NCU");
   out_at.set_entry(r, c++, "STDEV");
   out_at.set_entry(r, c++, "MIN");
   out_at.set_entry(r, c++, "P10");
   out_at.set_entry(r, c++, "P25");
   out_at.set_entry(r, c++, "P50");
   out_at.set_entry(r, c++, "P75");
   out_at.set_entry(r, c++, "P90");
   out_at.set_entry(r, c++, "MAX");
   out_at.set_entry(r, c++, "IQR");
   out_at.set_entry(r, c++, "RANGE");
   out_at.set_entry(r, c++, "SUM");
   out_at.set_entry(r, c++, "TS_INT");
   out_at.set_entry(r, c++, "TS_IND");
   out_at.set_entry(r, c++, "FSP_TOTAL");
   out_at.set_entry(r, c++, "FSP_BEST");
   out_at.set_entry(r, c++, "FSP_TIES");
   out_at.set_entry(r, c++, "FSP");

   // Compute the frequency of superior performance
   compute_fsp(fsp_total, fsp_best, fsp_ties);

   // Loop over the map entries and populate the output table
   for(it=SummaryMap.begin(),r=1; it!=SummaryMap.end(); it++,r++) {

      // Initialize column index
      c = 0;

      // Split the current map key
      sa = it->first.split(":");

      // Get the valid subset of data
      v.clear();
      init.clear();
      lead.clear();
      valid.clear();
      for(i=0; i<it->second.Val.n_elements(); i++) {
         if(!is_bad_data(it->second.Val[i])) {
            v.add(it->second.Val[i]);
            init.add(it->second.Init[i]);
            lead.add(it->second.Lead[i]);
            valid.add(it->second.Valid[i]);
         }
      }

      // Build index array
      index.clear();
      for(i=0; i<v.n_elements(); i++) index.add(i);

      // Compute mean and standard deviation
      compute_mean_stdev(v, index, 1, OutAlpha, mean_ci, stdev_ci);

      // Compute the FSP value
      if(fsp_total[r-1] != 0) fsp = fsp_best[r-1]/fsp_total[r-1];
      else                    fsp = bad_data_double;

      // Compute time to independence for time-series data
      if(is_time_series(init, lead, valid, dsec)) {
         tind = compute_time_to_indep(v, dsec);
      }
      else {
         tind = dsec = bad_data_int;
      }

      // Write the table row
      out_at.set_entry(r, c++, "SUMMARY:");
      out_at.set_entry(r, c++, sa[0]);

      // Write case column values
      for(i=1; i<sa.n_elements(); i++)
         out_at.set_entry(r, c++, sa[i]);

      out_at.set_entry(r, c++, it->second.Val.n_elements());
      out_at.set_entry(r, c++, v.n_elements());
      out_at.set_entry(r, c++, mean_ci.v);
      out_at.set_entry(r, c++, mean_ci.v_ncl[0]);
      out_at.set_entry(r, c++, mean_ci.v_ncu[0]);
      out_at.set_entry(r, c++, stdev_ci.v);
      out_at.set_entry(r, c++, v.min());
      out_at.set_entry(r, c++, v.percentile_array(0.10));
      out_at.set_entry(r, c++, v.percentile_array(0.25));
      out_at.set_entry(r, c++, v.percentile_array(0.50));
      out_at.set_entry(r, c++, v.percentile_array(0.75));
      out_at.set_entry(r, c++, v.percentile_array(0.90));
      out_at.set_entry(r, c++, v.max());
      out_at.set_entry(r, c++, v.iqr());
      out_at.set_entry(r, c++, v.range());
      out_at.set_entry(r, c++, v.sum());
      if(is_bad_data(dsec)) out_at.set_entry(r, c++, dsec);
      else                  out_at.set_entry(r, c++, sec_to_hhmmss(dsec));
      if(is_bad_data(tind)) out_at.set_entry(r, c++, tind);
      else                  out_at.set_entry(r, c++, sec_to_hhmmss(tind));
      out_at.set_entry(r, c++, nint(fsp_total[r-1]));
      out_at.set_entry(r, c++, nint(fsp_best[r-1]));
      out_at.set_entry(r, c++, nint(fsp_ties[r-1]));
      out_at.set_entry(r, c++, fsp);
   }

   // Build a simple output line
   line << "JOB_LIST: " << serialize() << "\n";
   out  << line;

   // Write the table
   out << out_at << "\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::compute_fsp(NumArray &total, NumArray &best,
                                   NumArray &ties) {
   map<ConcatString,SummaryMapData,cs_cmp>::iterator it;
   StringArray case_list;
   double v, best_val;
   ConcatString best_mod, s;
   int i, j, k, n;

   // Initialize counts
   total.clear();
   best.clear();
   ties.clear();

   // Loop over the SummaryMap and initialize counts
   for(it=SummaryMap.begin(); it!=SummaryMap.end(); it++) {

      // Initialize counts to zero
      total.add(0);
      best.add(0);
      ties.add(0);
   } // end for it

   // Only compute FSP when AMODEL is in the case information
   if(!ByColumn.has("AMODEL")) {
      mlog << Debug(4)
           << "Skipping frequency of superior performance since "
           << "the case information does not contain \"AMODEL\".\n";
      return;
   }

   // Compute FSP cases as the intersection of the cases for each entry
   for(it=SummaryMap.begin(); it!=SummaryMap.end(); it++) {

      // Initialize case list to the first entry
      if(it==SummaryMap.begin()) {
         case_list = it->second.Hdr;
      }
      // Take the intersection of the remaining entries
      else {
         case_list = intersection(case_list, it->second.Hdr);
      }
   } // end for it

   mlog << Debug(4)
        << "Computing frequency of superior performance for "
        << Column.n_elements() << " columns and "
        << case_list.n_elements() << " cases.\n";

   // Loop over the columns being summarized
   for(i=0; i<Column.n_elements(); i++) {

      // Check if FSP should be computed for this column
      s = to_upper(Column[i]);
      if(strstr(s.c_str(), "-") == NULL && strstr(s.c_str(), "ERR") == NULL) {
         mlog << Debug(4)
              << "Skipping frequency of superior performance for "
              << "column \"" << Column[i] << "\" since it is not an "
              << "error or difference.\n";
         continue;
      }

      // Loop over the cases
      for(j=0; j<case_list.n_elements(); j++) {

         // Initialize top performer
         best_mod = na_str;
         best_val = bad_data_double;

         // Loop over the SummaryMap
         for(it=SummaryMap.begin(); it!=SummaryMap.end(); it++) {

            // Loop over the values for this entry
            for(k=0; k<it->second.Hdr.n_elements(); k++) {

               // Check if entry matches the current case
               if(strncasecmp(Column[i].c_str(), it->first.c_str(),
                              Column[i].length()) != 0 ||
                  case_list[j] != it->second.Hdr[k] )
                  continue;

               // Store the value
               v = it->second.Val[k];

               // Check for bad data
               if(is_bad_data(v)) continue;

               // Check for an meaningful improvment
               if(is_bad_data(best_val) ||
                  FSPThresh.check(fabs(best_val) - fabs(v))) {
                  best_val = v;
                  best_mod = it->second.AModel[k];
               }
               // Check for ties
               else if(fabs(v) <= fabs(best_val)) {
                  best_val = v;
                  best_mod = "TIE";
               }

            } // end for k
         } // end for it

         // Check for no top performer found
         if(is_bad_data(best_val)) {
            mlog << Debug(4)
                 << "For case \"" << Column[i] << ":" << case_list[j]
                 << "\" no superior performance found.\n";
            continue;
         }

         mlog << Debug(4)
              << "For case \"" << Column[i] << ":" << case_list[j]
              << "\" superior performance of " << best_val
              << " by \"" << best_mod << "\".\n";

         // Loop over the SummaryMap to update counts
         for(it=SummaryMap.begin(),n=0; it!=SummaryMap.end(); it++,n++) {

            // Check if entry has the current case
            if(strncasecmp(Column[i].c_str(), it->first.c_str(),
                           Column[i].length()) != 0 ||
               !it->second.Hdr.has(case_list[j]))
               continue;

            // Update total
            total.set(n, total[n]+1);

            // See if this entry is the top performer
            if( best_mod == it->second.AModel[0] )
               best.set(n, best[n]+1);

            // See if there was a tie
            if( best_mod == "TIE" )
               ties.set(n, ties[n]+1);

         } // end for it

      } // end for j
   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

TCStatJobType string_to_tcstatjobtype(const ConcatString s) {
   TCStatJobType t;

        if(strcasecmp(s.c_str(), TCStatJobType_FilterStr)   == 0) t = TCStatJobType_Filter;
   else if(strcasecmp(s.c_str(), TCStatJobType_SummaryStr)  == 0) t = TCStatJobType_Summary;
   else if(strcasecmp(s.c_str(), TCStatJobType_RIRWStr)     == 0) t = TCStatJobType_RIRW;
   else if(strcasecmp(s.c_str(), TCStatJobType_ProbRIRWStr) == 0) t = TCStatJobType_ProbRIRW;
   else                                                           t = NoTCStatJobType;

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString tcstatjobtype_to_string(const TCStatJobType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case TCStatJobType_Filter:   s = TCStatJobType_FilterStr;   break;
      case TCStatJobType_Summary:  s = TCStatJobType_SummaryStr;  break;
      case TCStatJobType_RIRW:     s = TCStatJobType_RIRWStr;     break;
      case TCStatJobType_ProbRIRW: s = TCStatJobType_ProbRIRWStr; break;
      default:                     s = na_str;                    break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////
//
// A time-series exists if either the init, valid, or lead time are
// fixed and the other two entries vary by a fixed amount.
//
////////////////////////////////////////////////////////////////////////

bool is_time_series(const TimeArray &init, const NumArray &lead,
                    const TimeArray &valid, int &dsec) {
   int i, dinit, dlead, dvalid;

   // Initialize
   dsec = bad_data_int;

   // The arrays should all be of the same length > 1
   if(init.n_elements() != lead.n_elements() ||
      init.n_elements() != valid.n_elements() ||
      init.n_elements() < 2) {
      mlog << Debug(4)
           << "Skipping time-series computations since the array "
           << "lengths differ.\n";
      return(false);
   }

   // Initialize time spacing
   dinit  = init[1] - init[0];
   dlead  = nint(lead[1] - lead[0]);
   dvalid = valid[1] - valid[0];

   // Loop over the entries to determine the time spacing
   for(i=0; i<init.n_elements()-1; i++) {

      // Make sure the time spacing remains fixed
      if(dinit!= (init[i+1] - init[i])) {
         mlog << Debug(4)
              << "Skipping time-series computations since the "
              << "initialization time spacing changed: " << dinit
              << " != " << (init[i+1] - init[i]) << "\n";
         return(false);
      }
      else if(dlead != (lead[i+1] - lead[i])) {
         mlog << Debug(4)
              << "Skipping time-series computations since the "
              << "lead time spacing changed: " << dlead
              << " != " << (lead[i+1] - lead[i]) << "\n";
         return(false);
      }
      else if(dvalid != (valid[i+1] - valid[i])) {
         mlog << Debug(4)
              << "Skipping time-series computations since the "
              << "valid time spacing changed: " << dvalid
              << " != " << (valid[i+1] - valid[i]) << "\n";
         return(false);
      }
   }

   // Check for one fixed and the others varying by a non-zero amount
   if(dinit == 0 && dlead == dvalid && dlead > 0) {
      dsec = dlead;
      mlog << Debug(4)
           << "Computing time-series for initialization time \""
           << unix_to_yyyymmdd_hhmmss(init[0]) << "\" and spacing \""
           << sec_to_hhmmss(dsec) << "\".\n";
   }
   else if(dlead == 0 && dvalid == dinit && dvalid > 0) {
      dsec = dvalid;
      mlog << Debug(4)
           << "Computing time-series for valid time \""
           << unix_to_yyyymmdd_hhmmss(valid[0]) << "\" and spacing \""
           << sec_to_hhmmss(dsec) << "\".\n";
   }
   else if(dvalid == 0 && dinit == dlead && dinit > 0) {
      dsec = dinit;
      mlog << Debug(4)
           << "Computing time-series for lead time \""
           << sec_to_hhmmss(nint(lead[0])) << "\" and spacing \""
           << sec_to_hhmmss(dsec) << "\".\n";
   }

   return(!is_bad_data(dsec));
}

////////////////////////////////////////////////////////////////////////

int compute_time_to_indep(const NumArray &val, int ds) {
   double mean, exp_runs, eff_size, tind;
   int i, n_abv, n_run_abv, n_bel, n_run_bel;
   bool cur_abv, prv_abv;

   // Compute the mean value of the time-series
   mean = val.mean();

   // Initialize
   n_abv = n_run_abv = 0;
   n_bel = n_run_bel = 0;
   prv_abv = false;

   // Count the number of values above and belwo the mean
   for(i=0; i<val.n_elements(); i++) {

      // Store the current state
      cur_abv = (val[i] >= mean);

      // Increment counters
      if(cur_abv) n_abv++;
      else        n_bel++;

      // Count runs above and below
      if(i == 1) {
         if(cur_abv) n_run_abv++;
         else        n_run_bel++;
      }
      else {
              if(cur_abv  && !prv_abv) n_run_abv++;
         else if(!cur_abv &&  prv_abv) n_run_bel++;
      }

      // Store previous state
      prv_abv = cur_abv;

   } // end for i

   // Calculate expected number of runs
   exp_runs = 1.0 + 2.0*(n_abv * n_bel)/(n_abv + n_bel);

   // Calculate effective sample size, time to independence
   eff_size = val.n_elements()*(n_run_abv + n_run_bel)/exp_runs;
   tind     = ds*val.n_elements()/eff_size;

   return(nint(tind));
}

////////////////////////////////////////////////////////////////////////

StringArray intersection(const StringArray &s1, const StringArray &s2) {
   StringArray s;
   int i;

   // Add elements common to both list
   for(i=0; i<s1.n_elements(); i++) {
      if(s2.has(s1[i])) s.add(s1[i]);
   }

   return(s);
}

////////////////////////////////////////////////////////////////////////

void parse_thresh_option(const char *col_name, const char *col_val,
                         map<ConcatString,ThreshArray> &m) {
   ConcatString cs = to_upper((string)col_name);
   ThreshArray ta;
   ta.add_css(col_val);

   // Add data to the map
   if(m.count(cs) > 0) m[(string)col_name].add(ta);
   else                m.insert(pair<ConcatString, ThreshArray>(cs, ta));

   return;
}

////////////////////////////////////////////////////////////////////////

void parse_string_option(const char *col_name, const char *col_val,
                         map<ConcatString,StringArray> &m) {
   ConcatString cs = to_upper((string)col_name);
   StringArray sa;
   sa.set_ignore_case(1);
   sa.add_css(col_val);

   // Add data to the map
   if(m.count(cs) > 0) m[(string)col_name].add(sa);
   else                m.insert(pair<ConcatString, StringArray>(cs, sa));

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class TCStatJobRIRW
//
////////////////////////////////////////////////////////////////////////

TCStatJobRIRW::TCStatJobRIRW() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCStatJobRIRW::~TCStatJobRIRW() {

   clear();
}

////////////////////////////////////////////////////////////////////////

TCStatJobRIRW::TCStatJobRIRW(const TCStatJobRIRW &j) {

   init_from_scratch();

   assign(j);
}

////////////////////////////////////////////////////////////////////////

TCStatJobRIRW & TCStatJobRIRW::operator=(const TCStatJobRIRW &j) {

   if(this == &j) return(*this);

   assign(j);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::init_from_scratch() {
   int i;

   for(i=0; i<4; i++) DumpOutCTC[i] = (ofstream *) 0;

   TCStatJob::init_from_scratch();

   // Ignore case when performing comparisons
   ByColumn.set_ignore_case(1);

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::clear() {
   int i;

   TCStatJob::clear();

   JobType = TCStatJobType_RIRW;

   // Disable rapid intensification/weakening filtering logic.
   RIRWTrack = TrackType_None;

   ByColumn.clear();
   RIRWMap.clear();

   for(i=0; i<4; i++) DumpFileCTC[i].clear();
   close_dump_file();

   // Set to default values
   OutAlpha = default_tc_alpha;
   OutLineType.clear();
   OutLineType.add(stat_ctc_str);
   OutLineType.add(stat_cts_str);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::assign(const TCStatJobRIRW & j) {

   TCStatJob::assign(j);

   ByColumn    = j.ByColumn;
   OutAlpha    = j.OutAlpha;
   OutLineType = j.OutLineType;
   RIRWMap     = j.RIRWMap;

   return;
}

////////////////////////////////////////////////////////////////////////

StringArray TCStatJobRIRW::parse_job_command(const char *jobstring) {
   StringArray a, b;
   string c;
   int i;

   // Call the parent and store any unused options
   a = TCStatJob::parse_job_command(jobstring);

   // Clear the OutLineType if the user has specified it
   if(a.has("-out_line_type")) OutLineType.clear();

   // Loop over the StringArray elements
   for(i=0; i<a.n_elements(); i++) {

      // Point at the current entry
      c = to_lower(a[i]);

      // Check for a job command option
      if(c[0] != '-') {
         b.add(a[i]);
         continue;
      }

      // Check job command options
           if(c.compare("-by"            ) == 0) { ByColumn.add_css(to_upper(a[i+1]));            a.shift_down(i, 1); }
      else if(c.compare("-out_alpha"     ) == 0) { OutAlpha = atof(a[i+1].c_str());               a.shift_down(i, 1); }
      else if(c.compare("-out_line_type" ) == 0) { OutLineType.add_css(to_upper(a[i+1]).c_str()); a.shift_down(i, 1); }
      else                                       {                                                b.add(a[i]);        }
   }

   return(b);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::open_dump_file() {
   const char *category[] = { "FY_OY", "FY_ON", "FN_OY", "FN_ON" };
   int i;

   close_dump_file();

   if(DumpFile.empty()) return;

   // Use DumpFile entry to construct output file names for each category
   for(i=0; i<4; i++) {

      DumpFileCTC[i] << cs_erase << DumpFile << "_"  << category[i] << ".tcst";
      DumpOutCTC[i] = new ofstream;
      DumpOutCTC[i]->open(DumpFileCTC[i].c_str());

      if(!DumpOutCTC[i]) {
         mlog << Error << "\nTCStatJob::open_dump_file()-> "
              << "can't open the output file \"" << DumpFileCTC[i]
              << "\" for writing!\n\n";
         exit(1);
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::close_dump_file() {
   int i;

   for(i=0; i<4; i++) {
      if(DumpOutCTC[i]) {
         mlog << Debug(1)
              << "Creating output dump file: " << DumpFileCTC[i] << "\n";

         DumpOutCTC[i]->close();
         delete DumpOutCTC[i];
         DumpOutCTC[i] = (ofstream *) 0;
      }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString TCStatJobRIRW::serialize() const {
   ConcatString s;
   int i;

   // Initialize the jobstring
   s.set_precision(get_precision());

   // Call the parent
   s = TCStatJob::serialize();

   // List the ADeck and BDeck rapid intensification logic
   if(RIRWTimeADeck == RIRWTimeBDeck) {
      s << "-rirw_time " << sec_to_hhmmss(RIRWTimeADeck) << " ";
   }
   else {
      s << "-rirw_time_adeck " << sec_to_hhmmss(RIRWTimeADeck) << " "
        << "-rirw_time_bdeck " << sec_to_hhmmss(RIRWTimeBDeck) << " ";
   }
   if(RIRWExactADeck == RIRWExactBDeck) {
      s << "-rirw_exact " << bool_to_string(RIRWExactADeck) << " ";
   }
   else {
      s << "-rirw_exact_adeck " << bool_to_string(RIRWExactADeck) << " "
        << "-rirw_exact_bdeck " << bool_to_string(RIRWExactBDeck) << " ";
   }
   if(RIRWThreshADeck == RIRWThreshBDeck) {
      s << "-rirw_thresh " << RIRWThreshADeck.get_str() << " ";
   }
   else {
      s << "-rirw_thresh_adeck " << RIRWThreshADeck.get_str() << " "
        << "-rirw_thresh_bdeck " << RIRWThreshBDeck.get_str() << " ";
   }
   s << "-rirw_window " << sec_to_hhmmss(RIRWWindowBeg) << " "
     << sec_to_hhmmss(RIRWWindowEnd) << " ";

   // Add RIRW job-specific options
   for(i=0; i<ByColumn.n_elements(); i++)
      s << "-by " << ByColumn[i] << " ";

   for(i=0; i<OutLineType.n_elements(); i++)
      s << "-out_line_type " << OutLineType[i] << " ";

   // Always list the output alpha value used
   s << "-out_alpha " << OutAlpha << " ";

   return(s);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::do_job(const StringArray &file_list,
                           TCLineCounts &n) {
   TrackPairInfo pair;

   // Add the input file list
   TCSTFiles.add_files(file_list);

   // Apply the event equalization logic to build a list of common cases
   if(EventEqual == true) event_equalize_tracks();

   // Check for no common cases
   if(EventEqualSet == true && EventEqualCases.n_elements() == 0) {
      mlog << Debug(1)
           << "Event equalization found no common cases.\n";
   }
   // Otherwise, process the track data
   else {

      // Rewind to the beginning of the track pair input
      TCSTFiles.rewind();

      // Process each of the track pairs
      while(TCSTFiles >> pair) {

         // Process the track pair down to points to be used
         subset_track_pair(pair, n);

         if(pair.n_points() > 0) {

            mlog << Debug(4)
                 << "Processing pair: " << pair.case_info() << "\n";

            // Process the track pair info for the RI/RW job
            process_pair(pair);
         }
      } // end while
   } // end else

   // Close the dump file
   close_dump_file();

   // Process the RI/RW job output
   if(JobOut) do_output(*JobOut);
   else       do_output(cout);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::process_pair(TrackPairInfo &pair) {
   int i, j, a, b, cur_dt, min_dt, i_min_dt, lead;
   map<ConcatString,RIRWMapData,cs_cmp> cur_map;
   ConcatString key, cur, cat;
   RIRWMapData data;
   TrackPairInfo pair_pt;
   int acur, aprv, adlt;
   int bcur, bprv, bdlt;
   const char *sep = ":";

   // Initialize the map
   cur_map.clear();

   // Apply the rapid intensification/weakening logic
   pair.check_rirw(TrackType_Both,
                   RIRWTimeADeck, RIRWTimeBDeck,
                   RIRWExactADeck, RIRWExactBDeck,
                   RIRWThreshADeck, RIRWThreshBDeck);

   // Loop over the track points and populate the contigency table
   for(i=0; i<pair.n_points(); i++) {

      // Build the map key
      key = build_map_key("RIRW", *pair.line(i), ByColumn);

      // Add map entry for this key, if necessary
      if(cur_map.count(key) == 0) cur_map[key] = data;

      // Get the ADeck and BDeck events
      a = nint(pair.adeck_rirw(i));
      b = nint(pair.bdeck_rirw(i));

      // Store current info
      lead = pair.adeck()[i].lead();
      acur = pair.adeck()[i].v_max();
      aprv = pair.adeck_prv_int(i);
      adlt = (is_bad_data(acur) || is_bad_data(aprv) ? bad_data_int : acur - aprv);
      bcur = pair.bdeck()[i].v_max();
      bprv = pair.bdeck_prv_int(i);
      bdlt = (is_bad_data(bcur) || is_bad_data(bprv) ? bad_data_int : bcur - bprv);

      // Check time window when the ADECK and BDECK are both valid but disagree.
      // Try to switch misses to hits and false alarms to correct negatives.
      if(!is_bad_data(a) && !is_bad_data(b) && a != b &&
         (RIRWWindowBeg != 0 || RIRWWindowEnd != 0)) {

         // Loop through the ADECK track points
         for(j=0, min_dt=bad_data_int, i_min_dt=bad_data_int; j<pair.n_points(); j++) {

            // Skip ADECK points that are too far away in time
            cur_dt = pair.valid(i) - pair.valid(j);
            if(cur_dt < RIRWWindowBeg || cur_dt > RIRWWindowEnd) continue;

            // Skip ADECK points that don't agree with the BDECK category
            if(nint(pair.adeck_rirw(j)) != b) continue;

            // Find closest match in time
            if(is_bad_data(min_dt))              { min_dt = cur_dt; i_min_dt = j; }
            else if(labs(cur_dt) < labs(min_dt)) { min_dt = cur_dt; i_min_dt = j; }
         } // end for j

         // Switch the category if a match was found within the window
         if(!is_bad_data(min_dt)) {
            a = b;
            acur = pair.adeck()[i_min_dt].v_max();
            aprv = pair.adeck_prv_int(i_min_dt);
            adlt = acur - aprv;
            mlog << Debug(4)
                 << "Switching "
                 << (b == 1 ? "FN_OY" : "FY_ON") << " to "
                 << (b == 1 ? "FY_OY" : "FN_ON")
                 << " since ADECK "  << (b == 1 ? "event" : "non-event")
                 << " (" << aprv << " to " << acur << " = " << adlt << ") occurred at "
                 << unix_to_yyyymmdd_hhmmss(pair.valid(i_min_dt)) << " ("
                 << sec_to_hhmmss(min_dt) << " offset within "
                 << sec_to_hhmmss(RIRWWindowBeg) << " to " << sec_to_hhmmss(RIRWWindowEnd)
                 << " window) for " << pair.case_info() << ", "
                 << "VALID = " << unix_to_yyyymmdd_hhmmss(pair.valid(i)) << ", "
                 << "LEAD = " << (is_bad_data(lead) ? na_str : sec_to_hhmmss(lead).text()) << "\n";
         }
      }

      // Determine the contingency table category
      if(is_bad_data(a) || is_bad_data(b)) {
         cat = na_str;
      }
      else {

         cat << cs_erase
             << "F"  << (a == 1 ? "Y" : "N")
             << "_O" << (b == 1 ? "Y" : "N");

              if(a == 1 && b == 1) cur_map[key].Info.cts.inc_fy_oy();
         else if(a == 1 && b == 0) cur_map[key].Info.cts.inc_fy_on();
         else if(a == 0 && b == 1) cur_map[key].Info.cts.inc_fn_oy();
         else if(a == 0 && b == 0) cur_map[key].Info.cts.inc_fn_on();

         // Write the current point to the correct dump file
         if(DumpOutCTC[0]) {

            // Create a track for the current point
            pair_pt.clear();
            pair_pt.add(*pair.line(i));

                 if(a == 1 && b == 1) dump_pair(pair_pt, DumpOutCTC[0]);
            else if(a == 1 && b == 0) dump_pair(pair_pt, DumpOutCTC[1]);
            else if(a == 0 && b == 1) dump_pair(pair_pt, DumpOutCTC[2]);
            else if(a == 0 && b == 0) dump_pair(pair_pt, DumpOutCTC[3]);
         }
      }

      // Add values for this key:
      //    AMODEL        BMODEL        STORM_ID
      //    INIT          LEAD          VALID
      //    AMAX_WIND_PRV AMAX_WIND_CUR AMAX_WIND_DLT ARIRW
      //    BMAX_WIND_PRV BMAX_WIND_CUR BMAX_WIND_DLT BRIRW
      //    CATEGORY
      cur << cs_erase
          << pair.line(i)->amodel() << sep
          << pair.line(i)->bmodel() << sep
          << pair.line(i)->storm_id() << sep
          << unix_to_yyyymmdd_hhmmss(pair.line(i)->init()) << sep
          << (is_bad_data(lead) ? na_str : sec_to_hhmmss(lead).text()) << sep
          << unix_to_yyyymmdd_hhmmss(pair.line(i)->valid()) << sep
          << aprv << sep << acur << sep << adlt << sep
          << (is_bad_data(a) ? na_str : bool_to_string(a)) << sep
          << bprv << sep << bcur << sep << bdlt << sep
          << (is_bad_data(b) ? na_str : bool_to_string(b)) << sep
          << cat;
      cur_map[key].Hdr.add(cur);
   } // end for i

   // Add the current map
   add_map(cur_map);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::add_map(map<ConcatString,RIRWMapData,cs_cmp>&m) {
   map<ConcatString,RIRWMapData,cs_cmp>::iterator it;

   // Loop over the input map entries
   for(it=m.begin(); it!= m.end(); it++) {

      // Current map key is not yet defined in RIRWMap
      if(RIRWMap.count(it->first) == 0) {

         mlog << Debug(5)
              << "RIRW Map Insert (" << it->first << ") "
              << it->second.Info.cts.total() << " contingency table entries: "
              << it->second.Info.cts.fy_oy() << " Hits, "
              << it->second.Info.cts.fy_on() << " False Alarms, "
              << it->second.Info.cts.fn_oy() << " Misses, "
              << it->second.Info.cts.fn_on() << " Correct Negatives.\n";

         // Add the pair to the map
         RIRWMap[it->first] = it->second;
      }
      // Current map key is defined in RIRWMap
      else {

         mlog << Debug(5)
              << "RIRW Map Add (" << it->first << ") "
              << it->second.Info.cts.total() << " contingency table entries: "
              << it->second.Info.cts.fy_oy() << " hits, "
              << it->second.Info.cts.fy_on() << " false alarms, "
              << it->second.Info.cts.fn_oy() << " misses, "
              << it->second.Info.cts.fn_on() << " correct negatives.\n";

         // Increment the counts for the existing key
         RIRWMap[it->first].Info.cts.set_fy_oy(
            RIRWMap[it->first].Info.cts.fy_oy() +
            it->second.Info.cts.fy_oy());
         RIRWMap[it->first].Info.cts.set_fy_on(
            RIRWMap[it->first].Info.cts.fy_on() +
            it->second.Info.cts.fy_on());
         RIRWMap[it->first].Info.cts.set_fn_oy(
            RIRWMap[it->first].Info.cts.fn_oy() +
            it->second.Info.cts.fn_oy());
         RIRWMap[it->first].Info.cts.set_fn_on(
            RIRWMap[it->first].Info.cts.fn_on() +
            it->second.Info.cts.fn_on());
         RIRWMap[it->first].Hdr.add(it->second.Hdr);
      }
   } // end for it

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::do_output(ostream &out) {
   ConcatString line;

   // Build a simple output line
   line << "JOB_LIST: " << serialize() << "\n";
   out  << line;

   if(OutLineType.has(stat_ctc_str)) do_ctc_output(out);
   if(OutLineType.has(stat_cts_str)) do_cts_output(out);
   if(OutLineType.has(stat_mpr_str)) do_mpr_output(out);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::do_ctc_output(ostream &out) {
   map<ConcatString,RIRWMapData,cs_cmp>::iterator it;
   StringArray sa;
   int i, r, c;
   AsciiTable out_at;

   // Format the output table
   out_at.set_size((int) RIRWMap.size() + 1,
                   9 + ByColumn.n_elements() + n_ctc_columns);
   setup_table(out_at, 9 + ByColumn.n_elements(), get_precision());

   // Initialize row and column indices
   r = c = 0;

   // Write the header row
   out_at.set_entry(r, c++, "COL_NAME:");

   // Write the RI/RW event definition headers
   out_at.set_entry(r, c++, "ATIME");
   out_at.set_entry(r, c++, "BTIME");
   out_at.set_entry(r, c++, "AEXACT");
   out_at.set_entry(r, c++, "BEXACT");
   out_at.set_entry(r, c++, "ATHRESH");
   out_at.set_entry(r, c++, "BTHRESH");
   out_at.set_entry(r, c++, "WINDOW_BEG");
   out_at.set_entry(r, c++, "WINDOW_END");

   // Write case column names
   for(i=0; i<ByColumn.n_elements(); i++) {
      out_at.set_entry(r, c++, ByColumn[i]);
   }

   // Write the header columns
   write_header_row(ctc_columns, n_ctc_columns, 0, out_at, r, c);

   // Loop over the map entries and populate the output table
   for(it=RIRWMap.begin(),r=1; it!=RIRWMap.end(); it++,r++) {

      // Split the current map key
      sa = it->first.split(":");

      // Initialize column counter
      c = 0;

      // Write the table row
      out_at.set_entry(r, c++, "RIRW_CTC:");

      // Write the RI/RW event definition
      out_at.set_entry(r, c++, sec_to_hhmmss(RIRWTimeADeck));
      out_at.set_entry(r, c++, sec_to_hhmmss(RIRWTimeBDeck));
      out_at.set_entry(r, c++, bool_to_string(RIRWExactADeck));
      out_at.set_entry(r, c++, bool_to_string(RIRWExactBDeck));
      out_at.set_entry(r, c++, RIRWThreshADeck.get_str());
      out_at.set_entry(r, c++, RIRWThreshBDeck.get_str());
      out_at.set_entry(r, c++, sec_to_hhmmss(RIRWWindowBeg));
      out_at.set_entry(r, c++, sec_to_hhmmss(RIRWWindowEnd));

      // Write case column values
      for(i=1; i<sa.n_elements(); i++) {
         out_at.set_entry(r, c++, sa[i]);
      }

      // Write the contingency table counts
      write_ctc_cols(it->second.Info, out_at, r, c);
   }

   // Write the table
   out << out_at << "\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::do_cts_output(ostream &out) {
   map<ConcatString,RIRWMapData,cs_cmp>::iterator it;
   StringArray sa;
   int i, r, c;
   AsciiTable out_at;

   // Format the output table
   out_at.set_size((int) RIRWMap.size() + 1,
                   9 + ByColumn.n_elements() + n_cts_columns);
   setup_table(out_at, 9 + ByColumn.n_elements(), get_precision());

   // Initialize row and column indices
   r = c = 0;

   // Write the header row
   out_at.set_entry(r, c++, "COL_NAME:");

   // Write the RI/RW event definition headers
   out_at.set_entry(r, c++, "ATIME");
   out_at.set_entry(r, c++, "BTIME");
   out_at.set_entry(r, c++, "AEXACT");
   out_at.set_entry(r, c++, "BEXACT");
   out_at.set_entry(r, c++, "ATHRESH");
   out_at.set_entry(r, c++, "BTHRESH");
   out_at.set_entry(r, c++, "WINDOW_BEG");
   out_at.set_entry(r, c++, "WINDOW_END");

   // Write case column names
   for(i=0; i<ByColumn.n_elements(); i++) {
      out_at.set_entry(r, c++, ByColumn[i]);
   }

   // Write the header columns
   write_header_row(cts_columns, n_cts_columns, 0, out_at, r, c);

   // Loop over the map entries and populate the output table
   for(it=RIRWMap.begin(),r=1; it!=RIRWMap.end(); it++,r++) {

      // Allocate a single alpha value and compute statistics
      it->second.Info.allocate_n_alpha(1);
      it->second.Info.alpha[0] = OutAlpha;

      // Compute the stats and confidence intervals
      it->second.Info.compute_stats();
      it->second.Info.compute_ci();

      // Split the current map key
      sa = it->first.split(":");

      // Initialize column counter
      c = 0;

      // Write the table row
      out_at.set_entry(r, c++, "RIRW_CTS:");

      // Write the RI/RW event definition
      out_at.set_entry(r, c++, sec_to_hhmmss(RIRWTimeADeck));
      out_at.set_entry(r, c++, sec_to_hhmmss(RIRWTimeBDeck));
      out_at.set_entry(r, c++, bool_to_string(RIRWExactADeck));
      out_at.set_entry(r, c++, bool_to_string(RIRWExactBDeck));
      out_at.set_entry(r, c++, RIRWThreshADeck.get_str());
      out_at.set_entry(r, c++, RIRWThreshBDeck.get_str());
      out_at.set_entry(r, c++, sec_to_hhmmss(RIRWWindowBeg));
      out_at.set_entry(r, c++, sec_to_hhmmss(RIRWWindowEnd));

      // Write case column values
      for(i=1; i<sa.n_elements(); i++) {
         out_at.set_entry(r, c++, sa[i]);
      }

      // Write the contingency table statistics
      write_cts_cols(it->second.Info, 0, out_at, r, c);
   }

   // Write the table
   out << out_at << "\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobRIRW::do_mpr_output(ostream &out) {
   map<ConcatString,RIRWMapData,cs_cmp>::iterator it;
   StringArray sa;
   int i, j, r, c;
   AsciiTable out_at;
   ConcatString cs;

   // Determine the required number of rows
   for(it=RIRWMap.begin(),r=0; it!=RIRWMap.end(); it++) {
      r += it->second.Hdr.n_elements();
   }

   // Format the output table
   out_at.set_size(r + 1,
                   9 + ByColumn.n_elements() + 15);
   setup_table(out_at, 9 + ByColumn.n_elements(), get_precision());

   // Initialize row and column indices
   r = c = 0;

   // Write the header row
   out_at.set_entry(r, c++, "COL_NAME:");

   // Write the RI/RW event definition headers
   out_at.set_entry(r, c++, "ATIME");
   out_at.set_entry(r, c++, "BTIME");
   out_at.set_entry(r, c++, "AEXACT");
   out_at.set_entry(r, c++, "BEXACT");
   out_at.set_entry(r, c++, "ATHRESH");
   out_at.set_entry(r, c++, "BTHRESH");
   out_at.set_entry(r, c++, "WINDOW_BEG");
   out_at.set_entry(r, c++, "WINDOW_END");

   // Write case column names
   for(i=0; i<ByColumn.n_elements(); i++) {
      out_at.set_entry(r, c++, ByColumn[i]);
   }

   // Write the header columns
   out_at.set_entry(r, c++, "AMODEL");
   out_at.set_entry(r, c++, "BMODEL");
   out_at.set_entry(r, c++, "STORM_ID");
   out_at.set_entry(r, c++, "INIT");
   out_at.set_entry(r, c++, "LEAD");
   out_at.set_entry(r, c++, "VALID");
   out_at.set_entry(r, c++, "AMAX_WIND_PRV");
   out_at.set_entry(r, c++, "AMAX_WIND_CUR");
   out_at.set_entry(r, c++, "AMAX_WIND_DLT");
   out_at.set_entry(r, c++, "ARIRW");
   out_at.set_entry(r, c++, "BMAX_WIND_PRV");
   out_at.set_entry(r, c++, "BMAX_WIND_CUR");
   out_at.set_entry(r, c++, "BMAX_WIND_DLT");
   out_at.set_entry(r, c++, "BRIRW");
   out_at.set_entry(r, c++, "CATEGORY");

   // Loop over the map entries and populate the output table
   for(it=RIRWMap.begin(),r=1; it!=RIRWMap.end(); it++) {

      // Loop through the current points
      for(i=0; i<it->second.Hdr.n_elements(); i++,r++) {

         // Initialize column counter
         c = 0;

         // Write the table row
         out_at.set_entry(r, c++, "RIRW_MPR:");

         // Write the RI/RW event definition
         out_at.set_entry(r, c++, sec_to_hhmmss(RIRWTimeADeck));
         out_at.set_entry(r, c++, sec_to_hhmmss(RIRWTimeBDeck));
         out_at.set_entry(r, c++, bool_to_string(RIRWExactADeck));
         out_at.set_entry(r, c++, bool_to_string(RIRWExactBDeck));
         out_at.set_entry(r, c++, RIRWThreshADeck.get_str());
         out_at.set_entry(r, c++, RIRWThreshBDeck.get_str());
         out_at.set_entry(r, c++, sec_to_hhmmss(RIRWWindowBeg));
         out_at.set_entry(r, c++, sec_to_hhmmss(RIRWWindowEnd));

         // Write case column values
         sa = it->first.split(":");
         for(j=1; j<sa.n_elements(); j++) {
            out_at.set_entry(r, c++, sa[j]);
         }

         // Write the header information
         cs = it->second.Hdr[i];
         sa = cs.split(":");
         for(j=0; j<sa.n_elements(); j++) {
            out_at.set_entry(r, c++, sa[j]);
         }
      }
   }

   // Write the table
   out << out_at << "\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for class TCStatJobProbRIRW
//
////////////////////////////////////////////////////////////////////////

TCStatJobProbRIRW::TCStatJobProbRIRW() {

   init_from_scratch();
}

////////////////////////////////////////////////////////////////////////

TCStatJobProbRIRW::~TCStatJobProbRIRW() {

   clear();
}

////////////////////////////////////////////////////////////////////////

TCStatJobProbRIRW::TCStatJobProbRIRW(const TCStatJobProbRIRW &j) {

   init_from_scratch();

   assign(j);
}

////////////////////////////////////////////////////////////////////////

TCStatJobProbRIRW & TCStatJobProbRIRW::operator=(const TCStatJobProbRIRW &j) {

   if(this == &j) return(*this);

   assign(j);

   return(*this);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobProbRIRW::init_from_scratch() {

   StatOut = (ofstream *) 0;

   TCStatJob::init_from_scratch();

   // Ignore case when performing comparisons
   ByColumn.set_ignore_case(1);

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobProbRIRW::clear() {

   TCStatJob::clear();

   JobType = TCStatJobType_ProbRIRW;

   ByColumn.clear();
   ProbRIRWMap.clear();

   // Set to default values
   ProbRIRWExact = default_probrirw_exact;
   ProbRIRWBDeltaThresh = default_probrirw_bdelta_thresh;
   ProbRIRWProbThresh = string_to_prob_thresh(default_probrirw_prob_thresh);
   MaxNThresh = 0;
   NDumpLines = 0;
   OutAlpha = default_tc_alpha;
   OutLineType.clear();
   OutLineType.add(stat_pct_str);
   OutLineType.add(stat_pstd_str);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobProbRIRW::assign(const TCStatJobProbRIRW & j) {

   TCStatJob::assign(j);

   ProbRIRWExact        = j.ProbRIRWExact;
   ProbRIRWBDeltaThresh = j.ProbRIRWBDeltaThresh;
   ProbRIRWProbThresh   = j.ProbRIRWProbThresh;
   ByColumn             = j.ByColumn;
   MaxNThresh           = j.MaxNThresh;
   NDumpLines           = j.NDumpLines;
   OutAlpha             = j.OutAlpha;
   OutLineType          = j.OutLineType;
   ProbRIRWMap          = j.ProbRIRWMap;

   return;
}

////////////////////////////////////////////////////////////////////////

StringArray TCStatJobProbRIRW::parse_job_command(const char *jobstring) {
   StringArray a, b;
   string c;
   int i;

   // Call the parent and store any unused options
   a = TCStatJob::parse_job_command(jobstring);

   // Clear options if the user has specified them
   if(a.has("-out_line_type"))        OutLineType.clear();
   if(a.has("-probrirw_prob_thresh")) ProbRIRWProbThresh.clear();

   // Loop over the StringArray elements
   for(i=0; i<a.n_elements(); i++) {

      // Point at the current entry
      c = to_lower(a[i]);

      // Check for a job command option
      if(c[0] != '-') {
         b.add(a[i]);
         continue;
      }

      // Check job command options
           if(c.compare("-by"                    ) == 0) { ByColumn.add_css(to_upper(a[i+1]));                            a.shift_down(i, 1); }
      else if(c.compare("-out_alpha"             ) == 0) { OutAlpha = atof(a[i+1].c_str());                               a.shift_down(i, 1); }
      else if(c.compare("-out_line_type"         ) == 0) { OutLineType.add_css(to_upper(a[i+1]));                         a.shift_down(i, 1); }
      else if(c.compare("-probrirw_exact"        ) == 0) { ProbRIRWExact = string_to_bool(a[i+1].c_str());                a.shift_down(i, 1); }
      else if(c.compare("-probrirw_bdelta_thresh") == 0) { ProbRIRWBDeltaThresh.set(a[i+1].c_str());                      a.shift_down(i, 1); }
      else if(c.compare("-probrirw_prob_thresh"  ) == 0) { ProbRIRWProbThresh.add(string_to_prob_thresh(a[i+1].c_str())); a.shift_down(i, 1); }
      else                                               {                                                                b.add(a[i]);        }
   }

   return(b);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobProbRIRW::close_dump_file() {

   // Close the current output dump file stream
   if(DumpOut) {
      DumpOut->close();
      delete DumpOut;
      DumpOut = (ofstream *) 0;
   }

   // Prepare nicely formatted AsciiTable object
   AsciiTable out_at;
   DataLine line;
   LineDataFile f;
   int row, col;

   // Initialize the output AsciiTable
   out_at.set_size(1 + NDumpLines,
                   n_tc_header_cols + get_n_prob_rirw_cols(MaxNThresh));

   // Write the PROBRIRW header row
   write_prob_rirw_header_row(1, MaxNThresh, out_at, 0, 0);

   // Setup the output AsciiTable
   justify_tc_stat_cols(out_at);
   out_at.set_precision(get_precision());
   out_at.set_bad_data_value(bad_data_double);
   out_at.set_bad_data_str(na_str);
   out_at.set_delete_trailing_blank_rows(1);

   // Open the dump file back up for reading
   if(!f.open(DumpFile.c_str())) {
      mlog << Error << "\nTCStatJobProbRIRW::close_dump_file() -> "
           << "can't open the dump file \"" << DumpFile
           << "\" for reading!\n\n";
      exit(1);
   }

   // Read and write each line to the AsciiTable
   row = 1;
   while(f >> line) {
      for(col=0; col<line.n_items(); col++) {
         out_at.set_entry(row, col, line.get_item(col));
      }
      row++;
   }

   // Close the dump file
   f.close();

   // Call parent to open the dump file back up
   TCStatJob::open_dump_file();

   // Write the reformatted AsciiTable
   *DumpOut << out_at;

   // Call parent to close the dump file
   TCStatJob::close_dump_file();

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString TCStatJobProbRIRW::serialize() const {
   ConcatString s;
   int i;

   // Initialize the jobstring
   s.set_precision(get_precision());

   // Call the parent
   s = TCStatJob::serialize();

   // List the probability threshold information
   s << "-probrirw_exact " << bool_to_string(ProbRIRWExact) << " ";
   s << "-probrirw_bdelta_thresh " << ProbRIRWBDeltaThresh.get_str() << " ";
   s << "-probrirw_prob_thresh " << prob_thresh_to_string(ProbRIRWProbThresh) << " ";

   // Add ProbRIRW job-specific options
   for(i=0; i<ByColumn.n_elements(); i++)
      s << "-by " << ByColumn[i] << " ";

   for(i=0; i<OutLineType.n_elements(); i++)
      s << "-out_line_type " << OutLineType[i] << " ";

   // Always list the output alpha value used
   s << "-out_alpha " << OutAlpha << " ";

   return(s);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobProbRIRW::do_job(const StringArray &file_list,
                             TCLineCounts &n) {
   ProbRIRWPairInfo pair;

   // Check that -probrirw_thresh has been supplied
   if(is_bad_data(ProbRIRWThresh)) {
      mlog << Error << "\nTCStatJobProbRIRW::do_job() -> "
           << "must use the -probrirw_thresh job command option "
           << "to define the forecast probabilities to be evaluated.\n\n";
      exit(1);
   }

   // Check that the -dump_row option has been supplied
   if(!DumpOut) {
      mlog << Error << "\nTCStatJobProbRIRW::do_job() -> "
           << "this function may only be called when using the "
           << "-dump_row option in the job command line:\n"
           << serialize() << "\n\n";
      exit(1);
   }

   // Add the input file list
   TCSTFiles.add_files(file_list);

   // Apply the event equalization logic to build a list of common cases
   if(EventEqual == true) event_equalize_lines();

   // Check for no common cases
   if(EventEqualSet == true && EventEqualCases.n_elements() == 0) {
      mlog << Debug(1)
           << "Event equalization found no common cases.\n";
   }
   // Otherwise, process the pair data
   else {

      // Rewind to the beginning of the track pair input
      TCSTFiles.rewind();

      // Process each of the track pairs
      while(TCSTFiles >> pair) {

         // Increment the read and keep counts
         n.NRead++;
         n.NKeep++;

         // Check if this line should be kept
         if(!is_keeper_line(pair.line(), n)) continue;

         mlog << Debug(4)
              << "Processing pair: " << pair.case_info() << "\n";

         // Process the ProbRIRWPairInfo
         process_pair(pair);

         if(DumpOut) {
            dump_line(pair.line(), DumpOut);
            NDumpLines++;
         }

      } // end while
   } // end else

   // Close the dump file
   close_dump_file();

   // Process the ProbRIRW job output
   if(JobOut) do_output(*JobOut);
   else       do_output(cout);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobProbRIRW::process_pair(ProbRIRWPairInfo &pair) {
   int i, n;
   double f, o;
   ConcatString key, cur;
   ProbRIRWMapData data;
   NumArray p_thresh;

   // Track the maximum number of thresholds
   n = atoi(pair.line().get_item("N_THRESH"));
   if(n > MaxNThresh) MaxNThresh = n;

   // Build the map key
   key = build_map_key("PROBRIRW", pair.line(), ByColumn);

   // Add map entry for this key, if necessary
   if(ProbRIRWMap.count(key) == 0) {
      for(i=0; i<ProbRIRWProbThresh.n_elements(); i++) {
         p_thresh.add(ProbRIRWProbThresh[i].get_value());
      }
      data.Info.clear();
      data.Info.pct.set_size(ProbRIRWProbThresh.n_elements() - 1);
      data.Info.pct.set_thresholds(p_thresh.vals());
      data.Info.fthresh = ProbRIRWProbThresh;
      data.Info.othresh = ProbRIRWBDeltaThresh;
      data.RIRWWindow = pair.prob_rirw().rirw_window();
      ProbRIRWMap[key] = data;
   }

   // Get the forecast probability
   f = get_probrirw_value(pair.line(), ProbRIRWThresh);

   // Check for bad data
   if(is_bad_data(f)) {
      mlog << Error << "\nvoid TCStatJobProbRIRW::process_pair(ProbRIRWPairInfo &pair) -> "
           << "bad probability value!\n\n";
      exit(1);
   }

   // Threshold BDELTA or BDELTA_MAX
   o = (ProbRIRWExact ?
        atof(pair.line().get_item("BDELTA")) :
        atof(pair.line().get_item("BDELTA_MAX")));

   // Increment the PCT table
   if(ProbRIRWMap[key].Info.othresh.check(o)) {
      ProbRIRWMap[key].Info.pct.inc_event(f);
   }
   else {
      ProbRIRWMap[key].Info.pct.inc_nonevent(f);
   }

   // Make sure the RIRW_WINDOW remains constant
   if(pair.prob_rirw().rirw_window() != ProbRIRWMap[key].RIRWWindow) {
      mlog << Error << "\nvoid TCStatJobProbRIRW::process_pair(ProbRIRWPairInfo &pair) -> "
           << "RIRW_WINDOW must remain constant ("
           << ProbRIRWMap[key].RIRWWindow << " != "
           << pair.prob_rirw().rirw_window()
           << ").  Try setting \"-by RIRW_WINDOW\" "
           << "or \"-column_thresh RIRW_WINDOW ==n\".\n\n";
      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobProbRIRW::do_output(ostream &out) {
   map<ConcatString,ProbRIRWMapData,cs_cmp>::iterator it;
   StringArray sa;
   int n, i, j, lt_cols, r, c;
   AsciiTable out_at;
   STATLineType out_lt;
   ConcatString cs;

   // Build a simple output line
   cs  << "JOB_LIST: " << serialize() << "\n";
   out << cs;

   // Determine the maximum dimension
   for(it=ProbRIRWMap.begin(), n=0; it!=ProbRIRWMap.end(); it++) {
      n = max(it->second.Info.pct.nrows() + 1, n);
   }

   // Loop over the output line types
   for(i=0; i<OutLineType.n_elements(); i++) {

      // Parse the output line type
      out_lt = string_to_statlinetype(OutLineType[i].c_str());

      // Write the header columns
           if(out_lt == stat_pct)  lt_cols = get_n_pct_columns (n);
      else if(out_lt == stat_pstd) lt_cols = get_n_pstd_columns(n);
      else if(out_lt == stat_prc)  lt_cols = get_n_prc_columns (n);
      else if(out_lt == stat_pjc)  lt_cols = get_n_pjc_columns (n);
      else {
         mlog << Error << "\nvoid TCStatJobProbRIRW::do_output(ostream &out) -> "
              << "unsupported output line type \"" << OutLineType[i] << "\"\n\n";
         exit(1);
      }

      // Initialize the output table
      out_at.clear();
      out_at.set_size((int) ProbRIRWMap.size() + 1,
                      5 + ByColumn.n_elements() + lt_cols);
      setup_table(out_at, 5 + ByColumn.n_elements(), get_precision());

      // Initialize row and column indices
      r = c = 0;

      // Write the header row
      cs << cs_erase << "PROBRIRW_" << OutLineType[i] << ":";
      out_at.set_entry(r, c++, cs);

      // Write the ProbRIRW event definition headers
      out_at.set_entry(r, c++, "THRESH");
      out_at.set_entry(r, c++, "EXACT");
      out_at.set_entry(r, c++, "BDELTA_THRESH");
      out_at.set_entry(r, c++, "PROB_THRESH");

      // Write case column names
      for(j=0; j<ByColumn.n_elements(); j++)
         out_at.set_entry(r, c++, ByColumn[j]);

      // Write the header columns
           if(out_lt == stat_pct)  write_pct_header_row (0, n, out_at, r, c);
      else if(out_lt == stat_pstd) write_pstd_header_row(0, n, out_at, r, c);
      else if(out_lt == stat_prc)  write_prc_header_row (0, n, out_at, r, c);
      else if(out_lt == stat_pjc)  write_pjc_header_row (0, n, out_at, r, c);

      // Loop over the map entries and populate the output table
      for(it=ProbRIRWMap.begin(),r=1; it!=ProbRIRWMap.end(); it++,r++) {

         // Split the current map key
         sa = it->first.split(":");

         // Initialize column counter
         c = 0;

         // Write the table row
         out_at.set_entry(r, c++, cs);

         // Write the ProbRIRW event definition
         out_at.set_entry(r, c++, ProbRIRWThresh);
         out_at.set_entry(r, c++, bool_to_string(ProbRIRWExact));
         out_at.set_entry(r, c++, ProbRIRWBDeltaThresh.get_str());
         out_at.set_entry(r, c++, prob_thresh_to_string(ProbRIRWProbThresh));

         // Write case column values
         for(j=1; j<sa.n_elements(); j++)
            out_at.set_entry(r, c++, sa[j]);

         // Compute PSTD statistics
         if(out_lt == stat_pstd) {
            it->second.Info.allocate_n_alpha(1);
            it->second.Info.alpha[0] = OutAlpha;
            it->second.Info.compute_stats();
            it->second.Info.compute_ci();
         }

         // Write output columns
              if(out_lt == stat_pct)  write_pct_cols (it->second.Info,    out_at, r, c);
         else if(out_lt == stat_pstd) write_pstd_cols(it->second.Info, 0, out_at, r, c);
         else if(out_lt == stat_prc)  write_prc_cols (it->second.Info,    out_at, r, c);
         else if(out_lt == stat_pjc)  write_pjc_cols (it->second.Info,    out_at, r, c);
      } // end for it

      // Write the table for the current output line type
      out << out_at << "\n" << flush;

   } // end for i

   return;
}

////////////////////////////////////////////////////////////////////////

void setup_table(AsciiTable &at, int n_hdr_cols, int prec) {
   int i;

   // Left-justify header columns and right-justify data columns
   for(i=0;          i<n_hdr_cols; i++) at.set_column_just(i, LeftJust);
   for(i=n_hdr_cols; i<at.ncols(); i++) at.set_column_just(i, RightJust);

   // Right-justify the first column
   at.set_column_just(0, RightJust);

   // Set the precision
   at.set_precision(prec);

   // Set the bad data value
   at.set_bad_data_value(bad_data_double);

   // Set the bad data string
   at.set_bad_data_str(na_str);

   // Don't write out trailing blank rows
   at.set_delete_trailing_blank_rows(1);

   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString build_map_key(const char *prefix, const TCStatLine &l,
                           const StringArray &case_cols) {
   int i, lead;
   ConcatString key, cur;

   // Initialize the map key with prefix
   if(prefix) key = prefix;

   // Build case information for the map key
   for(i=0; i<case_cols.n_elements(); i++) {

     cur = l.get(case_cols[i].c_str());

      // For bad data, use the NA string
      if(is_bad_data(atoi(cur.c_str()))) cur = na_string;

      // Special handling for lead time:
      // Switch 2-digit hours to 3-digit hours so that the
      // summary job output is sorted nicely.
      if(strcasecmp(case_cols[i].c_str(), "LEAD") == 0 &&
         cur != na_str &&
         abs(lead = timestring_to_sec(cur.c_str())) < 100*sec_per_hour) {

         // Handle positive and negative lead times
         key << (lead < 0 ? ":-0" : ":0")
             << sec_to_hhmmss(abs(lead));
      }
      // Otherwise, just append the current case info
      else {
         key << ":" << cur;
      }
   } // end for i

   return(key);
}

////////////////////////////////////////////////////////////////////////

bool check_masks(const MaskPoly &mask_poly, const Grid &mask_grid,
                 const MaskPlane &mask_area, double lat, double lon) {
   double grid_x, grid_y;

   //
   // Check polyline masking.
   //
   if(mask_poly.n_points() > 0) {
      if(!mask_poly.latlon_is_inside_dege(lat, lon)) {
         return false;
      }
   }

   //
   // Check grid masking.
   //
   if(mask_grid.nx() > 0 || mask_grid.ny() > 0) {
      mask_grid.latlon_to_xy(lat, -1.0*lon, grid_x, grid_y);
      if(grid_x < 0 || grid_x >= mask_grid.nx() ||
         grid_y < 0 || grid_y >= mask_grid.ny()) {
         return false;
      }

      //
      // Check area mask.
      //
      if(mask_area.nx() > 0 || mask_area.ny() > 0) {
         if(!mask_area.s_is_on(nint(grid_x), nint(grid_y))) {
            return false;
         }
      }
   }

   return true;
}

////////////////////////////////////////////////////////////////////////

double get_probrirw_value(const TCStatLine &line, double ProbRIRWThresh) {
   double p = bad_data_double;
   int i, n;
   ConcatString cs;

   // Only valid for the PROBRIRW line type
   if(line.type() != TCStatLineType_ProbRIRW) return(bad_data_double);

   // Get the number of threhsolds
   n = atoi(line.get_item("N_THRESH"));

   // Find the matching threshold and get the corresponding probability
   // Rescale probabilities from [0, 100] to [0, 1]
   for(i=1; i<=n; i++) {
      cs << cs_erase << "THRESH_" << i;
      if(is_eq(ProbRIRWThresh, atof(line.get_item(cs.c_str())))) {
         cs << cs_erase << "PROB_" << i;
         p = atof(line.get_item(cs.c_str())) / 100.0;
         break;
      }
   }

   return(p);
}

////////////////////////////////////////////////////////////////////////
