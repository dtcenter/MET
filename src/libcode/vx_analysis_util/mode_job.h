// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_JOB_H__
#define  __MODE_JOB_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "mode_line.h"
#include "mode_atts.h"
#include "by_case_info.h"

#include "vx_cal.h"
#include "vx_util.h"


////////////////////////////////////////////////////////////////////////


static const char underline_char = '-';

static const int  dump_mode_buffer_rows = 512;


////////////////////////////////////////////////////////////////////////


class BasicModeAnalysisJob {

   protected:

      virtual void init_from_scratch();

      virtual void assign_basic_job(const BasicModeAnalysisJob &);

      // Output file precision
      int precision;

   public:

      BasicModeAnalysisJob();
      virtual ~BasicModeAnalysisJob();
      BasicModeAnalysisJob(const BasicModeAnalysisJob &);
      BasicModeAnalysisJob & operator=(const BasicModeAnalysisJob &);

      void set_precision (int);
      int  get_precision () const;

      virtual void clear();

      virtual void dump(ostream &, int depth = 0) const;


      virtual void add_column_by_name   (const char *);
      virtual void add_column_by_number (int);   //  numbers start at zero

      virtual void do_output(ostream &) const = 0;

      virtual void setup() = 0;

      virtual void do_job(const StringArray & mode_files) = 0;


      NumArray * accums;

      ModeAttributes atts;

      StringArray columns;

      int n_lines_read;
      int n_lines_kept;

      ostream * dumpfile;   //  NOT allocated, so don't delete
      ostream * outfile;    //  NOT allocated, so don't delete

      int        n_dump;    //  number of lines written to dump file
      AsciiTable dump_at;   //  AsciiTable object to buffer dump data

      virtual void dump_mode_line(const ModeLine &);

};


////////////////////////////////////////////////////////////////////////


inline void BasicModeAnalysisJob::set_precision (int p)  { precision = p; return; }

inline int  BasicModeAnalysisJob::get_precision () const { return(precision);     }


////////////////////////////////////////////////////////////////////////


class SummaryJob : public BasicModeAnalysisJob {

   protected:

      void init_from_scratch();

      void assign(const SummaryJob &);

   public:

      SummaryJob();
     ~SummaryJob();
      SummaryJob(const SummaryJob &);
      SummaryJob & operator=(const SummaryJob &);

      void clear();

      void do_output(ostream &) const;

      void setup();

      void do_job(const StringArray & mode_files);

      void process_mode_file(const char *);

};


////////////////////////////////////////////////////////////////////////


class ByCaseJob : public BasicModeAnalysisJob {

   protected:

      void init_from_scratch();

      void assign(const ByCaseJob &);

   public:

      ByCaseJob();
     ~ByCaseJob();
      ByCaseJob(const ByCaseJob &);
      ByCaseJob & operator=(const ByCaseJob &);

      void clear();

      void do_output(ostream &) const;

      void setup();

      void do_job(const StringArray & mode_files);

      void process_mode_file(const char * filename);

      ByCaseInfo * info;

      IntArray valid_times;

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_JOB_H__  */


////////////////////////////////////////////////////////////////////////


