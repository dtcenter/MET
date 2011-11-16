// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
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

#include "ascii_table.h"
#include "math_constants.h"

#include "analysis_utils.h"
#include "mode_job.h"
#include "mode_analysis_columns.h"
#include "by_case_info.h"


////////////////////////////////////////////////////////////////////////


static void get_times_from_file(const char * mode_filename, IntArray & valid_times, const int column_index, const ModeAttributes & atts);


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

accums = (NumArray *) 0;

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void BasicModeAnalysisJob::clear()

{

atts.clear();

columns.clear();

if ( accums )  { delete [] accums;  accums = (NumArray *) 0; }

n_lines_read = n_lines_kept = 0;

dumpfile = (ostream *) 0;   //  don't delete

outfile  = (ostream *) 0;   //  don't delete

return;

}


////////////////////////////////////////////////////////////////////////


void BasicModeAnalysisJob::assign_basic_job(const BasicModeAnalysisJob & a)

{

clear();

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

      out << "\"" << lc_mode_columns[columns[j]] << "\"";

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

int j;
int status;


for (j=0; j<n_mode_columns; ++j)  {

   // status = strcmp(lc_mode_columns[j], name);

   status = strcasecmp(lc_mode_columns[j], name);   //  case-insensitive comparison

   if ( status == 0 )  {

      add_column_by_number(j);

      return;

   }

}

cerr << "\n\n  BasicModeAnalysisJob::add_column_by_name(const char *) -> bad column name \"" << name << "\"\n\n";

exit ( 1 );


return;

}


////////////////////////////////////////////////////////////////////////


void BasicModeAnalysisJob::add_column_by_number(int k)

{

if ( (k < 0) || (k >= n_mode_columns) )  {

   cerr << "\n\n  BasicModeAnalysisJob::add_column_by_number(int) -> bad column number -> " << k << "\n\n";

   exit ( 1 );

}

columns.add(k);

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

int j, k, r, m;
double mean, stdev;
char junk[256];
AsciiTable table;
const int ncols = 12;


table.set_size(Nfields + 2, ncols);

table.set_bad_data_value(bad_data_double);
table.set_bad_data_str(na_str);

table.set_ics(3);

k = 0;

r = 0;

table.set_entry(r, k++, "Field");
table.set_entry(r, k++, "N");
table.set_entry(r, k++, "Min");
table.set_entry(r, k++, "Max");
table.set_entry(r, k++, "Mean");
table.set_entry(r, k++, "StdDev");
table.set_entry(r, k++, "P10");
table.set_entry(r, k++, "P25");
table.set_entry(r, k++, "P50");
table.set_entry(r, k++, "P75");
table.set_entry(r, k++, "P90");
table.set_entry(r, k++, "Sum");


for (j=0; j<Nfields; ++j)  {

   k = 0;

   r = j + 2;

   table.set_entry(r, k++, lc_mode_columns[columns[j]]);
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

for (j=0; j<ncols; ++j)  {

   m = table.col_width(j);

   memset(junk, 0, sizeof(junk));

   for (k=0; k<m; ++k)  junk[k] = underline_char;

   table.set_entry(1, j, junk);

};




   //
   //  done
   //

out << table;

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
char junk[256];
AsciiTable t;


setup();

if ( Nfields == 0 )  {

   cerr << "\n\n  do_job() -> no columns specified!\n\n";

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

   process_mode_file(mode_files[j]);

}

t.set_size(2, 2);

t.set_entry(0, 0, "Total mode lines read = ");
comma_string(n_lines_read, junk);
t.set_entry(0, 1, junk);

t.set_entry(1, 0, "Total mode lines kept = ");
comma_string(n_lines_kept, junk);
t.set_entry(1, 1, junk);

cout << "\n";

cout << t;

cout << "\n";

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

int j, k;
double value;
const int N = columns.n_elements();
LineDataFile in;
ModeLine L;


if ( !(in.open(path)) )  {

   cerr << "\n\n  SummaryJob::process_mode_file(const char *) -> can't open mode file \""
        << path << "\" for reading\n\n";

   exit  ( 1 );

}


while ( in >> L )  {

   if ( L.n_items() == 0 )  continue;

   ++n_lines_read;

   if ( !(atts.is_keeper(L)) )  continue;

   ++n_lines_kept;

   if ( dumpfile )  (*dumpfile) << L;

      //
      //  accumulate the data
      //

   for (j=0; j<N; ++j)  {

      k = columns[j];

      value = atof(L.get_item(k));

      if ( value <= -9999.0 )   continue;   //  bad data flag

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

int j, k, m;
int r;
const int Ncols = 7;
const int Nrows = 2 + valid_times.n_elements();
char junk[256];
AsciiTable table;


table.set_size(Nrows, Ncols);

table.set_bad_data_value(bad_data_double);
table.set_bad_data_str(na_str);

table.set_ics(3);

   //
   //  header lines
   //

k = 0;

r = 0;

table.set_entry(r, k++, "Fcst Valid Time");
table.set_entry(r, k++, "Area Matched");
table.set_entry(r, k++, "Area Unmatched");
table.set_entry(r, k++, "# Fcst Matched");
table.set_entry(r, k++, "# Fcst Unmatched");
table.set_entry(r, k++, "# Obs Matched");
table.set_entry(r, k++, "# Obs Unmatched");


   //
   //  entries
   //

for (j=0; j<(valid_times.n_elements()); ++j)  {

   k = 0;

   r = j + 2;

   make_timestring(info[j].valid, junk);
   table.set_entry(r, k++, junk);

   sprintf(junk, "%.0f", info[j].area_matched);
   table.set_entry(r, k++, junk);

   sprintf(junk, "%.0f", info[j].area_unmatched);
   table.set_entry(r, k++, junk);

   sprintf(junk, "%d", info[j].n_fcst_matched);
   table.set_entry(r, k++, junk);

   sprintf(junk, "%d", info[j].n_fcst_unmatched);
   table.set_entry(r, k++, junk);

   sprintf(junk, "%d", info[j].n_obs_matched);
   table.set_entry(r, k++, junk);

   sprintf(junk, "%d", info[j].n_obs_unmatched);
   table.set_entry(r, k++, junk);

}   //  for j


   //
   //  underline
   //

for (j=0; j<Ncols; ++j)  {

   m = table.col_width(j);

   memset(junk, 0, sizeof(junk));

   for (k=0; k<m; ++k)  junk[k] = underline_char;

   table.set_entry(1, j, junk);

};



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

   cout << "\n*** WARNING: the -bycase job should always be run with "
        << "the -single option! ***\n\n";

}

   //
   // The ByCaseJob should be run with either -simple or -cluster set
   //

if ( !atts.is_simple_toggle_set )  {

   cout << "\n*** WARNING: the -bycase job should always be run with "
        << "the -simple or -cluster option! ***\n\n";

}

return;

}


////////////////////////////////////////////////////////////////////////


void ByCaseJob::do_job(const StringArray & mode_files)

{

int j;
const int Nfiles       = mode_files.n_elements();
const int column_index = fcst_valid_column;
char junk[256];
AsciiTable t;
int n_valid_times;


setup();

if ( Nfiles == 0 )  return;

   //
   //   get valid times
   //

valid_times.clear();

for (j=0; j<Nfiles; ++j)  {

   get_times_from_file(mode_files[j], valid_times, column_index, atts);

}

valid_times.sort_increasing();

n_valid_times = valid_times.n_elements();

// if ( n_valid_times == 0 )  return;

if ( info )  { delete [] info;  info = (ByCaseInfo *) 0; }

info = new ByCaseInfo [n_valid_times + 1];  //  in case n_valid_times is zero

for (j=0; j<n_valid_times; ++j)  {

   info[j].valid = (unixtime) valid_times[j];

}


// cout << "\n\n There are " << valid_times.n_elements() << " times\n\n" << flush;

   //
   //  loop through the files
   //

for (j=0; j<Nfiles; ++j)  {

   process_mode_file(mode_files[j]);

}

t.set_size(2, 2);

t.set_entry(0, 0, "Total mode lines read = ");
comma_string(n_lines_read, junk);
t.set_entry(0, 1, junk);

t.set_entry(1, 0, "Total mode lines kept = ");
comma_string(n_lines_kept, junk);
t.set_entry(1, 1, junk);

cout << "\n";

cout << t;

cout << "\n";

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
const char * c = (const char *) 0;
const int column_index = fcst_valid_column;


if ( !(in.open(mode_filename)) )  {

   cerr << "\n\n  process_mode_file() -> can't open mode file \""
        << mode_filename << "\" for reading\n\n";

   exit  ( 1 );

}

while ( in >> L )  {

   ++n_lines_read;

   if ( !(atts.is_keeper(L)) )  continue;

   ++n_lines_kept;

   if ( dumpfile )  (*dumpfile) << L;

   c = L.get_item(column_index);

   t = (int) timestring_to_unix(c);

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


void get_times_from_file(const char * mode_filename, IntArray & valid_times, const int column_index, const ModeAttributes & atts)

{

LineDataFile in;
ModeLine L;
int t;
const char * c = (const char *) 0;


if ( !(in.open(mode_filename)) )  {

   cerr << "\n\n  get_times_from_file() -> can't open mode file \""
        << mode_filename << "\" for reading\n\n";

   exit  ( 1 );

}


while ( in >> L )  {

   if ( !(atts.is_keeper(L)) )  continue;

   c = L.get_item(column_index);

   t = (int) timestring_to_unix(c);

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





