// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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
#include "vx_tc_util.h"
#include "vx_log.h"
#include "vx_util.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

static const char *TCStatJobType_FilterStr  = "filter";
static const char *TCStatJobType_SummaryStr = "summary";

////////////////////////////////////////////////////////////////////////
//
// Code for class TCStatJobFactory
//
////////////////////////////////////////////////////////////////////////

TCStatJob *TCStatJobFactory::new_tc_stat_job_type(const char *type_str) {
   TCStatJob *job = (TCStatJob *) 0;
   TCStatJobType type = NoTCStatJobType;
   
   // Determine the TCStatJobType
   type = string_to_tcstatjobtype(type_str);

   // Switch on job type and instantiate the appropriate class.
   // The TCStatJob object is allocated and needs to be deleted by caller.
   switch(type) {

      case TCStatJobType_Filter:
         job = new TCStatJobFilter;
         break;

      case TCStatJobType_Summary:
         job = new TCStatJobSummary;
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
   StringArray sa;
   ConcatString type_str = na_str;
   int i;

   // Parse the jobstring into an array
   sa.parse_wsss(jobstring);

   // Check if the job type is specified
   if(sa.has("-job", i)) type_str = sa[i+1];

   // Allocate a new job of the requested type
   job = new_tc_stat_job_type(type_str);

   // Parse the jobstring into the new job
   job->parse_job_command(jobstring);

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
   ColumnThreshName.set_ignore_case(1);
   ColumnStrName.set_ignore_case(1);

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::clear() {

   JobType = NoTCStatJobType;

   PairArray.clear();

   AModel.clear();
   BModel.clear();
   StormId.clear();
   Basin.clear();
   Cyclone.clear();
   StormName.clear();
   InitBeg  = InitEnd  = (unixtime) 0;
   ValidBeg = ValidEnd = (unixtime) 0;
   InitHH.clear();
   Lead.clear();
   InitMask.clear();
   ValidMask.clear();
   LineType.clear();
   ColumnThreshName.clear();
   ColumnThreshVal.clear();
   ColumnStrName.clear();
   ColumnStrVal.clear();
   
   DumpFile.clear();
   close_dump_file();
   JobOut = (ofstream *) 0;

   OutInitMask.clear();
   OutValidMask.clear();

   // Set to default values
   MatchPoints = default_match_points;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::assign(const TCStatJob & j) {

   clear();
   
   JobType = j.JobType;

   AModel = j.AModel;
   BModel = j.BModel;
   StormId = j.StormId;
   Basin = j.Basin;
   Cyclone = j.Cyclone;
   StormName = j.StormName;
   InitBeg  = j.InitBeg;
   InitEnd  = j.InitEnd;
   ValidBeg = j.ValidBeg;
   ValidEnd = j.ValidEnd;
   InitHH = j.InitHH;
   Lead = j.Lead;
   InitMask = j.InitMask;
   ValidMask = j.ValidMask;
   LineType = j.LineType;
   ColumnThreshName = j.ColumnThreshName;
   ColumnThreshVal = j.ColumnThreshVal;
   ColumnStrName = j.ColumnStrName;
   ColumnStrVal = j.ColumnStrVal;

   DumpFile = j.DumpFile;
   open_dump_file();

   OutInitMask = j.OutInitMask;
   OutValidMask = j.OutValidMask;

   MatchPoints = j.MatchPoints;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::dump(ostream & out, int depth) const {
   Indent prefix(depth);

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
   out << prefix << "ValidBeg = " << unix_to_yyyymmdd_hhmmss(ValidBeg) << "\n";
   out << prefix << "ValidEnd = " << unix_to_yyyymmdd_hhmmss(ValidEnd) << "\n";

   out << prefix << "InitHH ...\n";
   InitHH.dump(out, depth + 1);

   out << prefix << "Lead ...\n";
   Lead.dump(out, depth + 1);

   out << prefix << "InitMask ...\n";
   InitMask.dump(out, depth + 1);

   out << prefix << "ValidMask ...\n";
   ValidMask.dump(out, depth + 1);

   out << prefix << "LineType ...\n";
   LineType.dump(out, depth + 1);

   out << prefix << "ColumnThreshName ...\n";
   ColumnThreshName.dump(out, depth + 1);

   out << prefix << "ColumnThreshVal ...\n";
   ColumnThreshVal.dump(out, depth + 1);

   out << prefix << "ColumnStrName ...\n";
   ColumnStrName.dump(out, depth + 1);

   out << prefix << "ColumnStrVal ...\n";
   ColumnStrVal.dump(out, depth + 1);

   out << prefix << "DumpFile = " << (DumpFile ? DumpFile.text() : na_str) << "\n";

   out << prefix << "OutInitMask = " << (OutInitMask.name() ? OutInitMask.name() : na_str) << "\n";
   
   out << prefix << "OutValidMask = " << (OutValidMask.name() ? OutValidMask.name() : na_str) << "\n";

   out << prefix << "MatchPoints = " << bool_to_string(MatchPoints) << "\n";
   
   out.flush();

   return;
}

////////////////////////////////////////////////////////////////////////

bool TCStatJob::is_keeper(const TCStatLine &line, int &skip_lines,
                          TCLineCounts &n) const {
   bool keep = true;
   int i, j, offset;
   double v_dbl, alat, alon, blat, blon;
   ConcatString v_str;
   StringArray sa;

   // Check TC-STAT header columns
   if(AModel.n_elements() > 0 &&
     !AModel.has(line.amodel()))        { keep = false; n.RejAModel++;    }
   else if(BModel.n_elements() > 0 &&
     !BModel.has(line.bmodel()))        { keep = false; n.RejBModel++;    }
   else if(StormId.n_elements() > 0 &&
     !has_storm_id(StormId, line.basin(), line.cyclone(), line.init()))
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
   else if(InitHH.n_elements() > 0 &&
     !InitHH.has(line.init_hh()))       { keep = false; n.RejInitHH++;    }
   else if(Lead.n_elements() > 0 &&
     !Lead.has(line.lead()))            { keep = false; n.RejLead++;      }
   else if(ValidBeg > 0 &&
      line.valid() < ValidBeg)          { keep = false; n.RejValid++;     }
   else if(ValidEnd > 0 &&
      line.valid() > ValidEnd)          { keep = false; n.RejValid++;     }
   else if(InitMask.n_elements() > 0 &&
     !InitMask.has(line.init_mask()))   { keep = false; n.RejInitMask++;  }
   else if(ValidMask.n_elements() > 0 &&
     !ValidMask.has(line.valid_mask())) { keep = false; n.RejValidMask++; }
   else if(LineType.n_elements() > 0 &&
     !LineType.has(line.line_type()))   { keep = false; n.RejLineType++;  }

   // Check the numeric column thresholds
   if(keep == true) {

      // Loop through the numeric column thresholds
      for(i=0; i<ColumnThreshName.n_elements(); i++) {

         // Get the numeric column value
         v_dbl = get_column_double(line, ColumnThreshName[i]);

         // Check the column threshold
         if(is_bad_data(v_dbl) || !ColumnThreshVal[i].check(v_dbl)) {
           keep = false;
           n.RejColumnThresh++;
           break;
         }
      } // end for i
   }

   // Check the column string matching
   if(keep == true) {

      // Loop through the column string matching
      for(i=0; i<ColumnStrName.n_elements(); i++) {

         // Construct list of all entries for the current column name
         sa.clear();
         for(j=0; j<ColumnStrName.n_elements(); j++)
            if(strcasecmp(ColumnStrName[i], ColumnStrName[j]) == 0)
               sa.add(ColumnStrVal[j]);
        
         // Determine the column offset and retrieve the value
         offset = determine_column_offset(line.type(), ColumnStrName[i]);
         v_str  = line.get_item(offset);

         // Check the string value
         if(!sa.has(v_str)) {
            keep = false;
            n.RejColumnStr++;
            break;
         }
      } // end for i
   }

   // Retrieve the ADECK and BDECK lat/lon values
   alat = atof(line.get_item("ALAT"));
   alon = atof(line.get_item("ALON"));
   blat = atof(line.get_item("BLAT"));
   blon = atof(line.get_item("BLON"));

   // Initialize skip lines to 0
   skip_lines = 0;

   // Check OutInitMask.  Check the first valid ADECK lat/lon track point.
   // If it falls outside the polyline region, skip the entire track.
   if(OutInitMask.n_points() > 0 &&
      !is_bad_data(alat) &&
      !is_bad_data(alon)) {

      // Check ADECK locations
      if(is_bad_data(alat) || is_bad_data(alon) ||
         !OutInitMask.latlon_is_inside_dege(alat, alon)) {

         keep = false;
         n.RejOutInitMask++;

         // Skip the remainder of the track: TOTAL - INDEX
         skip_lines = atoi(line.get_item("TOTAL")) -
                      atoi(line.get_item("INDEX"));
      }
   }

   // Check OutValidMask
   if(keep == true && OutValidMask.n_points() > 0) {

      // Check ADECK locations
      if(is_bad_data(alat) ||
         is_bad_data(alon) ||
         !OutValidMask.latlon_is_inside_dege(alat, alon)) {
        
         keep = false;
         n.RejOutValidMask++;
      }
   }

   // Check MatchPoints
   if(keep == true && MatchPoints == true) {

      // Check that the ADECK and BDECK locations are both defined
      if(is_bad_data(alat) || is_bad_data(alon) ||
         is_bad_data(blat) || is_bad_data(blon)) {

         keep = false;
         n.RejMatchPoints++;
      }
   }   

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

   // Check for absolute value
   if(strncasecmp(column, "ABS", 3) == 0) {
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
   v = atof(line.get_item(sa[0]));

   // If multiple columns, compute the requested difference
   if(sa.n_elements() > 1) {

      // Loop through the column
      for(i=1; i<sa.n_elements(); i++) {

         // Get the current column value
         v_cur = atof(line.get_item(sa[i]));

         // Compute the difference, checking for bad data
         if(is_bad_data(v) || is_bad_data(v_cur)) v  = bad_data_double;
         else                                     v -= v_cur;
      }
   }

   // Apply absolute value, if requested
   if(abs_flag) v = fabs(v);

   return(v);
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::parse_job_command(const char *jobstring) {
   StringArray a;
   const char * c = (const char *) 0;
   int i;

   // Parse the jobstring into a StringArray
   a.parse_wsss(jobstring);

   // Loop over the StringArray elements
   for(i=0; i<a.n_elements(); i++) {

      c = a[i];

      // Check for a job command option
      if(c[0] != '-') continue;

      // Check job command options
           if(strcasecmp(c, "-job"             ) == 0) { JobType = string_to_tcstatjobtype(a[i+1]);        a.shift_down(i, 1); }
      else if(strcasecmp(c, "-amodel"          ) == 0) { AModel.add(a[i+1]);                               a.shift_down(i, 1); }
      else if(strcasecmp(c, "-bmodel"          ) == 0) { BModel.add(a[i+1]);                               a.shift_down(i, 1); }
      else if(strcasecmp(c, "-storm_id"        ) == 0) { StormId.add(a[i+1]);                              a.shift_down(i, 1); }
      else if(strcasecmp(c, "-basin"           ) == 0) { Basin.add(a[i+1]);                                a.shift_down(i, 1); }
      else if(strcasecmp(c, "-cyclone"         ) == 0) { Cyclone.add(a[i+1]);                              a.shift_down(i, 1); }
      else if(strcasecmp(c, "-storm_name"      ) == 0) { StormName.add(a[i+1]);                            a.shift_down(i, 1); }
      else if(strcasecmp(c, "-init_beg"        ) == 0) { InitBeg  = timestring_to_unix(a[i+1]);            a.shift_down(i, 1); }
      else if(strcasecmp(c, "-init_end"        ) == 0) { InitEnd  = timestring_to_unix(a[i+1]);            a.shift_down(i, 1); }
      else if(strcasecmp(c, "-valid_beg"       ) == 0) { ValidBeg = timestring_to_unix(a[i+1]);            a.shift_down(i, 1); }
      else if(strcasecmp(c, "-valid_end"       ) == 0) { ValidEnd = timestring_to_unix(a[i+1]);            a.shift_down(i, 1); }
      else if(strcasecmp(c, "-init_hh"         ) == 0) { InitHH.add(timestring_to_sec(a[i+1]));            a.shift_down(i, 1); }
      else if(strcasecmp(c, "-lead"            ) == 0) { Lead.add(timestring_to_sec(a[i+1]));              a.shift_down(i, 1); }
      else if(strcasecmp(c, "-init_mask"       ) == 0) { InitMask.add(a[i+1]);                             a.shift_down(i, 1); }
      else if(strcasecmp(c, "-valid_mask"      ) == 0) { ValidMask.add(a[i+1]);                            a.shift_down(i, 1); }
      else if(strcasecmp(c, "-line_type"       ) == 0) { LineType.add(a[i+1]);                             a.shift_down(i, 1); }
      else if(strcasecmp(c, "-column_thresh"   ) == 0) { ColumnThreshName.add(a[i+1]);
                                                         ColumnThreshVal.add(a[i+2]);                      a.shift_down(i, 2); }
      else if(strcasecmp(c, "-column_str"      ) == 0) { ColumnStrName.add(a[i+1]);
                                                         ColumnStrVal.add(a[i+2]);                         a.shift_down(i, 2); }
      else if(strcasecmp(c, "-dump_row"        ) == 0) { DumpFile = a[i+1]; open_dump_file();              a.shift_down(i, 1); }
      else if(strcasecmp(c, "-out_init_mask"   ) == 0) { set_mask(OutInitMask, a[i+1]);                    a.shift_down(i, 1); }
      else if(strcasecmp(c, "-out_valid_mask"  ) == 0) { set_mask(OutValidMask, a[i+1]);                   a.shift_down(i, 1); }
      else if(strcasecmp(c, "-match_points"    ) == 0) { MatchPoints = string_to_bool(a[i+1]);             a.shift_down(i, 1); }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::set_mask(MaskPoly &p, const char *file) {
  
   ConcatString poly_file = replace_path(file);
   p.clear();
   p.load(poly_file);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::open_dump_file() {

   close_dump_file();

   if(DumpFile.empty()) return;

   DumpOut = new ofstream;
   DumpOut->open(DumpFile);

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

void TCStatJob::write_dump_file() {
  
   if(!DumpOut) return;
   
   int i, i_row;
   TcHdrColumns tchc;
   AsciiTable out_at;
        
   // Initialize the output AsciiTable
   out_at.set_size(PairArray.n_points() + 1,
                   n_tc_header_cols + n_tc_mpr_cols);

   // Setup the output AsciiTable
   out_at.set_table_just(LeftJust);
   out_at.set_precision(default_precision);
   out_at.set_bad_data_value(bad_data_double);
   out_at.set_bad_data_str(na_str);
   out_at.set_delete_trailing_blank_rows(1);

   // Write the TCMPR header row
   write_tc_mpr_header_row(1, out_at, 0, 0);
   
   // Loop over and write the pairs
   for(i=0, i_row=1; i<PairArray.n_pairs(); i++) {

      // Setup header columns
      tchc.clear();
      tchc.set_adeck_model(PairArray[i].adeck().technique());
      tchc.set_bdeck_model(PairArray[i].bdeck().technique());
      tchc.set_basin(PairArray[i].bdeck().basin());
      tchc.set_cyclone(PairArray[i].bdeck().cyclone());
   
      if(OutInitMask.n_points() > 0)  tchc.set_init_mask(OutInitMask.name());
      else                            tchc.set_init_mask(na_str);
      if(OutValidMask.n_points() > 0) tchc.set_valid_mask(OutValidMask.name());
      else                            tchc.set_valid_mask(na_str);

      // Write the TrackPairInfo object
      write_tc_mpr_row(tchc, PairArray[i], out_at, i_row);
   }

   // Write the AsciiTable to the file
   *DumpOut << out_at;

   // Close the dump file
   close_dump_file();
   
   return;
}

////////////////////////////////////////////////////////////////////////

ConcatString TCStatJob::serialize() const {
   ConcatString s;
   int i;

   // Initialize the jobstring
   s.clear();
   s.set_precision(default_precision);

   if(JobType != NoTCStatJobType)
      s << "-job " << tcstatjobtype_to_string(JobType) << " ";
   for(i=0; i<AModel.n_elements(); i++)
      s << "-amodel " << AModel[i] << " ";
   for(i=0; i<BModel.n_elements(); i++)
      s << "-bmodel " << BModel[i] << " ";
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
   if(ValidBeg > 0)
      s << "-valid_beg " << unix_to_yyyymmdd_hhmmss(ValidBeg) << " ";
   if(ValidEnd > 0)
      s << "-valid_end " << unix_to_yyyymmdd_hhmmss(ValidEnd) << " ";
   for(i=0; i<InitHH.n_elements(); i++)
      s << "-init_hh " << sec_to_hhmmss(InitHH[i]) << " ";
   for(i=0; i<Lead.n_elements(); i++)
      s << "-lead " << sec_to_hhmmss(Lead[i]) << " ";
   for(i=0; i<InitMask.n_elements(); i++)
      s << "-init_mask " << InitMask[i] << " ";
   for(i=0; i<ValidMask.n_elements(); i++)
      s << "-valid_mask " << ValidMask[i] << " ";
   for(i=0; i<LineType.n_elements(); i++)
      s << "-line_type " << LineType[i] << " ";
   for(i=0; i<ColumnThreshName.n_elements(); i++)
      s << "-column_thresh " << ColumnThreshName[i] << " "
                             << ColumnThreshVal[i].get_str() << " ";
   for(i=0; i<ColumnStrName.n_elements(); i++)
      s << "-column_str " << ColumnStrName[i] << " "
                          << ColumnStrVal[i] << " ";
   if(DumpFile.length() > 0)
      s << "-dump_row " << DumpFile << " ";
   if(OutInitMask.n_points() > 0)
      s << "-out_init_mask " << OutInitMask.file_name() << " ";
   if(OutValidMask.n_points() > 0)
      s << "-out_valid_mask " << OutValidMask.file_name() << " ";
   if(MatchPoints != default_match_points)
      s << "-match_points " << bool_to_string(MatchPoints) << " ";
   
   return(s);
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::do_job(const StringArray &file_list,
                       TCLineCounts &n) {
   int i;
   
   // Loop through the input file list
   for(i=0; i<file_list.n_elements(); i++) {
      process_tc_stat_file(file_list[i], n);
   }

   // Write the dump file
   if(DumpOut) write_dump_file();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJob::process_tc_stat_file(const char *path, TCLineCounts &n) {
   LineDataFile in;
   TCStatLine line;
   TrackPairInfo tpi;
   int skip, index, i;

   // Open the data file
   if(!(in.open(path))) {
      mlog << Error << "\nTCStatJob::process_tc_stat_file(const char *path) -> "
           << "can't open file \"" << path << "\" for reading\n\n";
      exit(1);
   }

   mlog << Debug(3)
        << "Reading file: " << path << "\n";

   // Process each TC-STAT line
   while(in >> line) {

      n.NRead++;

      // Get the index value for the current line
      index = atoi(line.get_item("INDEX"));

      // Check for the first point of a track
      if(index == 1) {
            
         // Store the current pair, write it, and clear it
         if(tpi.n_points() > 0) {
            PairArray.add(tpi);
            tpi.clear();
         }
      }

      // Check if this line meets the job criteria
      if(is_keeper(line, skip, n)) {

         // Add line to the pair
         tpi.add(line);
              
         n.NKeep++;
      }
      else {

         // If skip lines is non-zero, clear the track and decrement count
         if(skip > 0) {
            tpi.clear();
            n.NKeep -= skip;
         }
              
         // Skip lines if necessary
         for(i=0; i<skip; i++) {
            in >> line;
            n.NRead++;
         }
      }
   }

   // Store the current pair and write it
   if(tpi.n_points() > 0) PairArray.add(tpi);
   
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

   // Check that the -dump_row option has been supplied
   if(!DumpOut) {
      mlog << Error << "\nTCStatJobFilter::do_job() -> "
           << "this function may only be called when using the "
           << "-dump_row option in the job command line: "
           << serialize() << "\n\n";
      exit(1);
   }

   // Call the parent's do_job() to filter the data
   TCStatJob::do_job(file_list, n);
   
   // Process the filter output
   if(JobOut) do_output(*JobOut);
   else       do_output(cout);

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

   clear();

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::clear() {

   TCStatJob::clear();

   JobType = TCStatJobType_Summary;

   ReqColumn.clear();
   Column.clear();
   Case.clear();
   SummaryMap.clear();

   // Set to default value
   OutAlpha = default_tc_alpha;
   
   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::assign(const TCStatJobSummary & j) {

   TCStatJob::assign(j);

   ReqColumn = j.ReqColumn;
   Column = j.Column;
   Case = j.Case;
   SummaryMap = j.SummaryMap;
   OutAlpha = j.OutAlpha;

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::parse_job_command(const char *jobstring) {
   StringArray a;
   const char * c = (const char *) 0;
   int i;

   // Call the parent
   TCStatJob::parse_job_command(jobstring);
   
   // Parse the jobstring into a StringArray
   a.parse_wsss(jobstring);

   // Loop over the StringArray elements
   for(i=0; i<a.n_elements(); i++) {

      c = a[i];

      // Check for a job command option
      if(c[0] != '-') continue;

      // Check job command options
           if(strcasecmp(c, "-column"   ) == 0) { ReqColumn.add(a[i+1]); add_column(a[i+1]); a.shift_down(i, 1); }
      else if(strcasecmp(c, "-by"       ) == 0) { Case.add(a[i+1]);                          a.shift_down(i, 1); }
      else if(strcasecmp(c, "-out_alpha") == 0) { OutAlpha = atof(a[i+1]);                   a.shift_down(i, 1); }
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::add_column(const char *col) {
   int i, i_wind;
   ConcatString s;

   //
   // Handle special column names
   //

   // Track errors
   if(strcasecmp(col, "TRACK") == 0) {
      for(i=0; i<n_tc_cols_track; i++) Column.add(tc_cols_track[i]);
   }
   // Wind errors
   else if(strcasecmp(col, "WIND") == 0) {
      for(i_wind=0; i_wind<NWinds; i_wind++) {
         for(i=0; i<n_tc_cols_wind; i++) {
            s << cs_erase << tc_cols_wind[i] << WindIntensity[i_wind];
            Column.add(s);
         }
      }
   }
   // Otherwise, just add the column name
   else {
      Column.add(col);
   }

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
   for(i=0; i<Case.n_elements(); i++)
      s << "-by " << Case[i] << " ";
   s << "-out_alpha " << OutAlpha;

   return(s);
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::do_job(const StringArray &file_list,
                             TCLineCounts &n) {
   int i;

   // Check that the -column option has been supplied
   if(Column.n_elements() == 0) {
      mlog << Error << "\nTCStatJobSummary::do_job() -> "
           << "this function may only be called when using the "
           << "-column option in the job command line: "
           << serialize() << "\n\n";
      exit(1);
   }

   // Loop through the input file list
   for(i=0; i<file_list.n_elements(); i++) {
      process_tc_stat_file(file_list[i], n);
   }

   // Write the dump file
   if(DumpOut) write_dump_file();

   // Process the summary output
   if(JobOut) do_output(*JobOut);
   else       do_output(cout);

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::process_tc_stat_file(const char *path,
                                            TCLineCounts &n) {
   LineDataFile in;
   TCStatLine line;
   TrackPairInfo tpi;
   int skip, index, i, j;
   map<ConcatString,NumArray,cs_cmp> cur_map;
   map<ConcatString,NumArray,cs_cmp>::iterator it;
   ConcatString key, cur;
   NumArray val_na;
   double val;
   
   // Open the data file
   if(!(in.open(path))) {
      mlog << Error << "\nTCStatJobSummary::process_tc_stat_file(const char *path) -> "
           << "can't open file \"" << path << "\" for reading\n\n";
      exit(1);
   }

   mlog << Debug(3)
        << "Reading file: " << path << "\n";

   // Process each TC-STAT line
   while(in >> line) {

      n.NRead++;

      // Get the index value for the current line
      index = atoi(line.get_item("INDEX"));

      // Check for the first point of a track
      if(index == 1) {

         if(tpi.n_points() > 0) {

            // Store the current map and clear it
            add_map(cur_map);
            cur_map.clear();

            // Store the current pair and clear it
            PairArray.add(tpi);
            tpi.clear();
         }
      }

      // Check if this line meets the job criteria
      if(is_keeper(line, skip, n)) {

         // Add line to the pair
         tpi.add(line);

         // Increment the count
         n.NKeep++;

         // Add summary info to the current map
         for(i=0; i<Column.n_elements(); i++) {

            // Build the key and get the current column value
            key = Column[i];
            val = get_column_double(line, Column[i]);

            // Add case information to the key
            for(j=0; j<Case.n_elements(); j++) {

               cur = line.get_item(Case[j]);

               // For lead time column, make sure hour is 3-digits
               if(strcasecmp(Case[j], "LEAD") == 0 &&
                  hhmmss_to_sec(cur) < 100*sec_per_hour)
                    key << ":0" << cur;
               else key << ":"  << cur;
            }
     
            // Key is not yet defined in the map
            if((it = cur_map.find(key)) == cur_map.end()) {
              
               // Add the pair to the map
               val_na.clear();
               val_na.add(val);
               cur_map[key] = val_na;
            }
            // Key is defined in the map
            else {
              
               // Add the value for the existing key
               it->second.add(val);
            }
         }
      }
      else {

         // Check for non-zero skip count
         if(skip > 0) {

            // Clear the current map
            cur_map.clear();
           
            // Clear the current pair
            tpi.clear();

            // Decrement the count
            n.NKeep -= skip;
         }

         // Skip lines if necessary
         for(i=0; i<skip; i++) {
            in >> line;
            n.NRead++;
         }
      }
   }

   // Check if there's data to store before returning
   if(tpi.n_points() > 0) {

      // Store the current map
      add_map(cur_map);

      // Store the current pair
      PairArray.add(tpi);
   }

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::add_map(map<ConcatString,NumArray,cs_cmp>&m) {
   map<ConcatString,NumArray,cs_cmp>::iterator it_in;
   map<ConcatString,NumArray,cs_cmp>::iterator it;

   // Loop over the input map entries
   for(it_in = m.begin(); it_in != m.end(); it_in++) {

      // Current map key is not yet defined in SummaryMap
      if((it = SummaryMap.find(it_in->first)) == SummaryMap.end()) {

         mlog << Debug(5)
              << "Summary Map Insert (" << it_in->first << ") "
              << it_in->second.n_elements() << " values: "
              << it_in->second.serialize() << "\n";

         // Add the pair to the map
         SummaryMap[it_in->first] = it_in->second;
      }
      // Current map key is defined in SummaryMap
      else {

         mlog << Debug(5)
              << "Summary Map Add (" << it_in->first << ") "
              << it_in->second.n_elements() << " values: "
              << it_in->second.serialize() << "\n";

         // Add the value for the existing key
         it->second.add(it_in->second);
      }
   } // end for it_in

   return;
}

////////////////////////////////////////////////////////////////////////

void TCStatJobSummary::do_output(ostream &out) {
   map<ConcatString,NumArray,cs_cmp>::iterator it;
   StringArray sa;
   ConcatString case_info, line;
   AsciiTable out_at;
   NumArray v, index;
   CIInfo mean_ci, stdev_ci;
   int i, r, c;

   // Setup the output table
   out_at.set_size((int) SummaryMap.size() + 1, 19);
   out_at.set_table_just(LeftJust);
   out_at.set_precision(default_precision);
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
   out_at.set_entry(r, c++, "CASE");
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
   out_at.set_entry(r, c++, "SUM");

   // Loop over the map entries and popluate the output table
   for(it=SummaryMap.begin(),r=1; it != SummaryMap.end(); it++) {

      // Initialize column index
      c = 0;

      // Split the current map key
      sa = it->first.split(":");

      // Rebuild the case info
      if(sa.n_elements() == 1) {
         case_info = "NA";
      }
      else {
         case_info = sa[1];
         for(i=2; i<sa.n_elements(); i++) case_info << ":" << sa[i];
      }

      // Get the valid subset of data
      v.clear();
      for(i=0; i<it->second.n_elements(); i++) {
         if(!is_bad_data(it->second[i])) v.add(it->second[i]);
      }

      // Build index array
      index.clear();
      for(i=0; i<v.n_elements(); i++) index.add(i);

      // Compute mean and standard deviation
      compute_mean_stdev(v, index, 1, OutAlpha, mean_ci, stdev_ci);
                                
      // Write the table row
      out_at.set_entry(r, c++, "SUMMARY:");      
      out_at.set_entry(r, c++, sa[0]);
      out_at.set_entry(r, c++, case_info);
      out_at.set_entry(r, c++, it->second.n_elements());
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
      out_at.set_entry(r, c++, v.sum());

      // Increment row count
      r++;
   }

   // Build a simple output line
   line << "JOB_LIST: " << serialize() << "\n";
   out  << line;

   // Write the table
   out << out_at << "\n" << flush;

   return;
}

////////////////////////////////////////////////////////////////////////
//
// Code for misc functions
//
////////////////////////////////////////////////////////////////////////

TCStatJobType string_to_tcstatjobtype(const char *s) {
   TCStatJobType t;

        if(strcasecmp(s, TCStatJobType_FilterStr) == 0)  t = TCStatJobType_Filter;
   else if(strcasecmp(s, TCStatJobType_SummaryStr) == 0) t = TCStatJobType_Summary;
   else                                              t = NoTCStatJobType;

   return(t);
}

////////////////////////////////////////////////////////////////////////

ConcatString tcstatjobtype_to_string(const TCStatJobType t) {
   const char *s = (const char *) 0;

   switch(t) {
      case TCStatJobType_Filter:  s = TCStatJobType_FilterStr;  break;
      case TCStatJobType_Summary: s = TCStatJobType_SummaryStr; break;
      default:                    s = na_str;                   break;
   }

   return(ConcatString(s));
}

////////////////////////////////////////////////////////////////////////
