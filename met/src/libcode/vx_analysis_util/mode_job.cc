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
#include <strings.h>
#include <cstdio>
#include <cmath>

#include "analysis_utils.h"
#include "mode_job.h"
#include "mode_columns.h"
#include "engine.h"
#include "by_case_info.h"

#include "vx_math.h"
#include "vx_util.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static void get_times_from_file(const char * mode_filename, IntArray & valid_times, const ModeAttributes & atts);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class BasicModeAnalysisJob
   //


////////////////////////////////////////////////////////////////////////


BasicModeAnalysisJob::BasicModeAnalysisJob()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


BasicModeAnalysisJob::~BasicModeAnalysisJob()

{

clear();

}


////////////////////////////////////////////////////////////////////////


BasicModeAnalysisJob::BasicModeAnalysisJob(const BasicModeAnalysisJob & aj)

{

init_from_scratch();

assign_basic_job(aj);

}


////////////////////////////////////////////////////////////////////////


BasicModeAnalysisJob & BasicModeAnalysisJob::operator=(const BasicModeAnalysisJob & aj)

{

if ( this == &aj )  return ( * this );

assign_basic_job(aj);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void BasicModeAnalysisJob::init_from_scratch()

{

accums   = (NumArray *) 0;

dumpfile = (ostream *)  0;   //  don't delete

outfile  = (ostream *)  0;   //  don't delete

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void BasicModeAnalysisJob::clear()

{

precision = default_precision;

atts.clear();

columns.clear();

if ( accums )  { delete [] accums;  accums = (NumArray *) 0; }

n_lines_read = n_lines_kept = 0;

// Write any remaning lines to the dump file
if ( dumpfile )  *(dumpfile) << dump_at;

dumpfile = (ostream *) 0;   //  don't delete

outfile  = (ostream *) 0;   //  don't delete

n_dump = 0;

dump_at.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void BasicModeAnalysisJob::assign_basic_job(const BasicModeAnalysisJob & a)

{

clear();

precision = a.precision;

atts = a.atts;

columns = a.columns;

if ( a.accums )  {

   accums = new NumArray [a.columns.n_elements()];

   int j;

   for (j=0; j<(a.columns.n_elements()); ++j)  {

      accums[j] = a.accums[j];

   }

}

n_lines_read = a.n_lines_read;
n_lines_kept = a.n_lines_kept;

dumpfile = a.dumpfile;

outfile = a.outfile;

n_dump = a.n_dump;

dump_at = a.dump_at;

return;

}


////////////////////////////////////////////////////////////////////////


void BasicModeAnalysisJob::dump(ostream & out, int depth) const

{

int j, n;
Indent prefix(depth);


   //
   //  columns
   //

n = columns.n_elements();

if ( n == 0 )  {

   out << prefix << "no columns selected\n";

} else {

   out << prefix << "columns = { ";

   for (j=0; j<n; ++j)  {

      out << "\"" << columns[j] << "\"";

      if ( j < (n - 1) )  out << ", ";

   }

   out << " }\n";

}

   //
   //  mode attributes
   //

out << prefix << "BasicMode Attributes ...\n";

atts.dump(out, depth + 1);


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void BasicModeAnalysisJob::add_column_by_name(const char * name)

{

columns.add(name);

return;

}


////////////////////////////////////////////////////////////////////////


void BasicModeAnalysisJob::add_column_by_number(int k)

{

   //
   // Assume the column number corresponds to the current MET version
   //

if ( (k < 0) || (k >= (n_mode_hdr_columns + n_mode_obj_columns) ) )  {

   mlog << Error << "\nBasicModeAnalysisJob::add_column_by_number(int) -> "
        << "bad column number -> " << k << "\n\n";

   exit ( 1 );

}

if ( k < n_mode_hdr_columns )  {
   columns.add(mode_hdr_columns[k]);
}
else  {
   columns.add(mode_obj_columns[k - n_mode_hdr_columns]);
}

return;

}


////////////////////////////////////////////////////////////////////////


void BasicModeAnalysisJob::dump_mode_line(const ModeLine &L)

{

int j;

// Nothing to do with no dump file
if ( !dumpfile )  return;

// Write header before the first line
if ( n_dump == 0 )  {

   // Format the dump row AsciiTable
   dump_at.set_size(dump_mode_buffer_rows,
                    n_mode_hdr_columns + n_mode_obj_columns);
   justify_mode_cols(dump_at);
   dump_at.set_precision(get_precision());
   dump_at.set_bad_data_value(bad_data_double);
   dump_at.set_bad_data_str(na_str);
   dump_at.set_delete_trailing_blank_rows(1);

   // Write out the MODE header columns
   for (j=0; j<n_mode_hdr_columns; ++j)  {
      dump_at.set_entry(0, j, mode_hdr_columns[j]);
   }

   // Write out the MODE objects columns
   for (j=0; j<n_mode_obj_columns; ++j)  {
      dump_at.set_entry(0, j + n_mode_hdr_columns, mode_obj_columns[j]);
   }

   n_dump++;
}

// Store the current data line
for (j=0; j<L.n_items(); ++j)  {
  dump_at.set_entry(n_dump%dump_at.nrows(), j, (string)L.get_item(j));
}
n_dump++;

// Write the buffer, if full
if ( n_dump%dump_at.nrows() == 0 ) {
   *(dumpfile) << dump_at;
   dump_at.erase();
}

   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class SummaryJob
   //


////////////////////////////////////////////////////////////////////////


SummaryJob::SummaryJob()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


SummaryJob::~SummaryJob()

{

clear();

}


////////////////////////////////////////////////////////////////////////


SummaryJob::SummaryJob(const SummaryJob & job)

{

init_from_scratch();

assign(job);

}


////////////////////////////////////////////////////////////////////////


SummaryJob & SummaryJob::operator=(const SummaryJob & job)

{

if ( this == &job )  return ( * this );

assign(job);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void SummaryJob::init_from_scratch()

{

BasicModeAnalysisJob::init_from_scratch();

return;

}


////////////////////////////////////////////////////////////////////////


void SummaryJob::clear()

{

BasicModeAnalysisJob::clear();

return;

}


////////////////////////////////////////////////////////////////////////


void SummaryJob::assign(const SummaryJob & job)

{

clear();

BasicModeAnalysisJob::assign_basic_job(job);

return;

}


////////////////////////////////////////////////////////////////////////


void SummaryJob::do_output(ostream & out) const

{

const int Nfields = columns.n_elements();

if ( Nfields == 0 )  return;

int j, k, r;
double mean, stdev;
AsciiTable table;
const int ncols = 12;


table.set_size(Nfields + 2, ncols);

table.set_column_just(0, LeftJust);
for (j=1; j<table.ncols(); ++j) table.set_column_just(j, RightJust);

table.set_bad_data_value(bad_data_double);
table.set_bad_data_str(na_str);
table.set_delete_trailing_blank_rows(1);
table.set_ics(3);


k = 0;

r = 0;

table.set_entry(r, k++, (string)"Field");
table.set_entry(r, k++, (string)"N");
table.set_entry(r, k++, (string)"Min");
table.set_entry(r, k++, (string)"Max");
table.set_entry(r, k++, (string)"Mean");
table.set_entry(r, k++, (string)"StdDev");
table.set_entry(r, k++, (string)"P10");
table.set_entry(r, k++, (string)"P25");
table.set_entry(r, k++, (string)"P50");
table.set_entry(r, k++, (string)"P75");
table.set_entry(r, k++, (string)"P90");
table.set_entry(r, k++, (string)"Sum");


for (j=0; j<Nfields; ++j)  {

   k = 0;

   r = j + 2;

   table.set_entry(r, k++, (string)columns[j]);
   table.set_entry(r, k++, accums[j].n_elements());
   table.set_entry(r, k++, accums[j].min());
   table.set_entry(r, k++, accums[j].max());

   accums[j].compute_mean_stdev(mean, stdev);
   table.set_entry(r, k++, mean);
   table.set_entry(r, k++, stdev);

   table.set_entry(r, k++, accums[j].percentile_array(0.10));
   table.set_entry(r, k++, accums[j].percentile_array(0.25));
   table.set_entry(r, k++, accums[j].percentile_array(0.50));
   table.set_entry(r, k++, accums[j].percentile_array(0.75));
   table.set_entry(r, k++, accums[j].percentile_array(0.90));
   table.set_entry(r, k++, accums[j].sum());

}

/*
for (j=0; j<ncols; ++j)  {

   m = table.col_width(j);

   memset(junk, 0, sizeof(junk));

   for (k=0; k<m; ++k)  junk[k] = underline_char;

   table.set_entry(1, j, junk);

};
*/

table.line_up_decimal_points();

table.underline_row(1, underline_char);


   //
   //  done
   //

out << table << flush;

return;

}


////////////////////////////////////////////////////////////////////////


void SummaryJob::setup()

{

return;

}


////////////////////////////////////////////////////////////////////////


void SummaryJob::do_job(const StringArray & mode_files)

{

int j;
const int Nfields = columns.n_elements();
const int Nfiles  = mode_files.n_elements();
ConcatString junk;
AsciiTable t;


setup();

if ( Nfields == 0 )  {

   mlog << Error << "\nSummaryJob::do_job() -> "
        << "no columns specified!\n\n";

   exit ( 1 );

}


if ( Nfiles == 0 )  return;

   //
   //  allocate the accumulation registers
   //

if ( Nfields > 0 )  accums = new NumArray [Nfields];

   //
   //  loop through the files
   //

for (j=0; j<Nfiles; ++j)  {

   process_mode_file(mode_files[j].c_str());

}

comma_string(n_lines_read, junk);
mlog << Debug(2) << "Total mode lines read = " << junk << "\n";

comma_string(n_lines_kept, junk);
mlog << Debug(2) << "Total mode lines kept = " << junk << "\n";

   //
   //  output
   //

if ( outfile )  do_output(*outfile);
else            do_output(cout);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void SummaryJob::process_mode_file(const char * path)

{

int j;
double value;
const int N = columns.n_elements();
LineDataFile in;
ModeLine L;


if ( !(in.open(path)) )  {

   mlog << Error << "\nSummaryJob::process_mode_file(const char *) -> "
        << "can't open mode file \"" << path << "\" for reading\n\n";

   exit  ( 1 );

}


while ( in >> L )  {

   if ( L.is_header() )  continue;

   ++n_lines_read;

   if ( !(atts.is_keeper(L)) )  continue;

   ++n_lines_kept;

   dump_mode_line( L );

      //
      //  accumulate the data
      //

   for (j=0; j<N; ++j)  {

      value = atof(L.get_item(columns[j].c_str()));

      if ( is_bad_data(value) )   continue;

      accums[j].add(value);

   }   //  for j

}   //  while

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ByCaseJob
   //


////////////////////////////////////////////////////////////////////////


ByCaseJob::ByCaseJob()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ByCaseJob::~ByCaseJob()

{

clear();

}


////////////////////////////////////////////////////////////////////////


ByCaseJob::ByCaseJob(const ByCaseJob & job)

{

init_from_scratch();

assign(job);

}


////////////////////////////////////////////////////////////////////////


ByCaseJob & ByCaseJob::operator=(const ByCaseJob & job)

{

if ( this == &job )  return ( * this );

assign(job);

return ( * this );

}


////////////////////////////////////////////////////////////////////////


void ByCaseJob::init_from_scratch()

{

info = (ByCaseInfo *) 0;

BasicModeAnalysisJob::init_from_scratch();

return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseJob::clear()

{

BasicModeAnalysisJob::clear();

if ( info )  { delete [] info;  info = (ByCaseInfo *) 0; }

valid_times.clear();

return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseJob::assign(const ByCaseJob & job)

{

clear();

BasicModeAnalysisJob::assign_basic_job(job);

valid_times = job.valid_times;

if ( job.info )  {

   int j, n;

   n = valid_times.n_elements();

   info = new ByCaseInfo [n];

   for (j=0; j<n; ++j)  {

      info[j] = job.info[j];

   }

}

return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseJob::do_output(ostream & out) const

{

if ( valid_times.n_elements() == 0 )  return;

int j, k;
int r;
const int Ncols = 7;
const int Nrows = 2 + valid_times.n_elements();
ConcatString junk;
AsciiTable table;


table.set_size(Nrows, Ncols);

table.set_column_just(0, LeftJust);
for (j=1; j<table.ncols(); ++j) table.set_column_just(j, RightJust);

table.set_bad_data_value(bad_data_double);
table.set_bad_data_str(na_str);
table.set_delete_trailing_blank_rows(1);
table.set_ics(3);

   //
   //  header lines
   //

k = 0;

r = 0;

table.set_entry(r, k++, (string)"Fcst Valid Time");
table.set_entry(r, k++, (string)"Area Matched");
table.set_entry(r, k++, (string)"Area Unmatched");
table.set_entry(r, k++, (string)"# Fcst Matched");
table.set_entry(r, k++, (string)"# Fcst Unmatched");
table.set_entry(r, k++, (string)"# Obs Matched");
table.set_entry(r, k++, (string)"# Obs Unmatched");


   //
   //  entries
   //

for (j=0; j<(valid_times.n_elements()); ++j)  {

   k = 0;

   r = j + 2;

   make_timestring(info[j].valid, junk);
   table.set_entry(r, k++, junk);

   junk.format("%.0f", info[j].area_matched);
   table.set_entry(r, k++, junk);

   junk.format("%.0f", info[j].area_unmatched);
   table.set_entry(r, k++, junk);

   junk.format("%d", info[j].n_fcst_matched);
   table.set_entry(r, k++, junk);

   junk.format("%d", info[j].n_fcst_unmatched);
   table.set_entry(r, k++, junk);

   junk.format("%d", info[j].n_obs_matched);
   table.set_entry(r, k++, junk);

   junk.format("%d", info[j].n_obs_unmatched);
   table.set_entry(r, k++, junk);

}   //  for j


   //
   //  underline
   //
/*
for (j=0; j<Ncols; ++j)  {

   m = table.col_width(j);

   memset(junk, 0, sizeof(junk));

   for (k=0; k<m; ++k)  junk[k] = underline_char;

   table.set_entry(1, j, junk);

};
*/

table.line_up_decimal_points();

table.underline_row(1, underline_char);


   //
   //  done
   //

out << table << flush;

return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseJob::setup()

{

   //
   // The ByCaseJob should always be run with -single set
   //

if ( !atts.is_single_toggle_set ||
     (atts.is_single_toggle_set && !atts.is_single) )  {

   mlog << Warning << "\nThe -bycase job should always be run with "
        << "the -single option!\n\n";

}

   //
   // The ByCaseJob should be run with either -simple or -cluster set
   //

if ( !atts.is_simple_toggle_set )  {

   mlog << Warning << "\nThe -bycase job should always be run with "
        << "the -simple or -cluster option!\n\n";

}

return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseJob::do_job(const StringArray & mode_files)

{

int j;
const int Nfiles = mode_files.n_elements();
ConcatString junk;
int n_valid_times;


setup();

if ( Nfiles == 0 )  return;

   //
   //   get valid times
   //

valid_times.clear();

for (j=0; j<Nfiles; ++j)  {

   get_times_from_file(mode_files[j].c_str(), valid_times, atts);

}

valid_times.sort_increasing();

n_valid_times = valid_times.n_elements();

if ( info )  { delete [] info;  info = (ByCaseInfo *) 0; }

info = new ByCaseInfo [n_valid_times + 1];  //  in case n_valid_times is zero

for (j=0; j<n_valid_times; ++j)  {

   info[j].valid = (unixtime) valid_times[j];

}


   //
   //  loop through the files
   //

for (j=0; j<Nfiles; ++j)  {

   process_mode_file(mode_files[j].c_str());

}

comma_string(n_lines_read, junk);
mlog << Debug(2) << "Total mode lines read = " << junk << "\n";

comma_string(n_lines_kept, junk);
mlog << Debug(2) << "Total mode lines kept = " << junk << "\n";

   //
   //  output
   //

if ( outfile )  do_output(*outfile);
else            do_output(cout);

   //
   //  done
   //

if ( info )  { delete [] info;  info = (ByCaseInfo *) 0; }

return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseJob::process_mode_file(const char * mode_filename)

{

const int N = valid_times.n_elements();

if ( N == 0 )  return;

LineDataFile in;
ModeLine L;
int t, index;


if ( !(in.open(mode_filename)) )  {

   mlog << Error << "\nByCaseJob::process_mode_file() -> "
        << "can't open mode file \"" << mode_filename << "\" for reading\n\n";

   exit  ( 1 );

}

while ( in >> L )  {

   if ( L.is_header() )  continue;

   ++n_lines_read;

   if ( !(atts.is_keeper(L)) )  continue;

   ++n_lines_kept;

   dump_mode_line( L );

   t = L.fcst_valid();

   if ( valid_times.has(t, index) )  {

      info[index].add(L);

   }

}   //  while


   //
   //  done
   //

in.close();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void get_times_from_file(const char * mode_filename, IntArray & valid_times, const ModeAttributes & atts)

{

LineDataFile in;
ModeLine L;
int t;


if ( !(in.open(mode_filename)) )  {

   mlog << Error << "\nget_times_from_file() -> "
        << "can't open mode file \"" << mode_filename << "\" for reading\n\n";

   exit  ( 1 );

}


while ( in >> L )  {

   if ( L.is_header() )  continue;

   if ( !(atts.is_keeper(L)) )  continue;

   t = L.fcst_valid();

   if ( !(valid_times.has(t)) )  {

      valid_times.add(t);

   }

}   //  while


   //
   //  done
   //

in.close();

return;

}


////////////////////////////////////////////////////////////////////////





