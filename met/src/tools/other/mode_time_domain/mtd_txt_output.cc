// ** National Center for Atmospheric Research (NCAR)
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "ascii_table.h"

#include "mtd_txt_output.h"
#include "3d_att.h"
#include "3d_txt_header.h"
#include "3d_single_columns.h"
#include "2d_columns.h"


////////////////////////////////////////////////////////////////////////


const int fcst_valid_column = 4;   //  0-based
const int  obs_valid_column = 6;   //  0-based

const int  fcst_lead_column = fcst_valid_column - 1;   //  0-based
const int   obs_lead_column =  obs_valid_column - 1;   //  0-based


////////////////////////////////////////////////////////////////////////


void do_3d_single_txt_output(const SingleAtt3DArray & fcst_att,
                             const SingleAtt3DArray &  obs_att,
                             const MtdConfigInfo & config,
                             const char * output_filename)

{

if ( !(config.do_3d_att_ascii) )  return;

int j, r, c;
ofstream out;
AsciiTable table;
const int Nobj = fcst_att.n() + obs_att.n();


out.open(output_filename);

if ( ! out )  {

   mlog << Error << "\n\n  do_3d_single_txt_output() -> unable to open output filename \""
        << output_filename << "\'\n\n";

   exit ( 1 );

}

table.set_size(1 + Nobj, n_header_3d_cols + n_3d_single_cols);

table.set_ics(2);

table.set_bad_data_value(bad_data_double);
table.set_bad_data_str(na_str);

   //
   //  column headings
   //

r = 0;

c = 0;

for (j=0; j<n_header_3d_cols; ++j)  {

   table.set_entry(r, c++, header_3d_cols[j]);

}

for (j=0; j<n_3d_single_cols; ++j)  {

   table.set_entry(r, c++, att_3d_single_cols[j]);

}

   //
   //  leading columns
   //

for (j=0; j<Nobj; ++j)  {

   r = j + 1;

   config.write_header_cols(table, r);

}

   //
   //  attributes
   //

r = 1;

for (j=0; j<(fcst_att.n()); ++j)  {

   fcst_att[j].write_txt(table, r++);

}

for (j=0; j<(obs_att.n()); ++j)  {

   obs_att[j].write_txt(table, r++);

}


   //
   //  done
   //

out << table;

out.close();

return;

}


////////////////////////////////////////////////////////////////////////


   //
   //  for single fields
   //


void do_3d_single_txt_output(const SingleAtt3DArray & att,
                             const MtdConfigInfo & config,
                             const char * output_filename)

{

if ( !(config.do_3d_att_ascii) )  return;

int j, r, c;
ofstream out;
AsciiTable table;
const int Nobj = att.n();


out.open(output_filename);

if ( ! out )  {

   mlog << Error << "\n\n  do_3d_single_txt_output[single]() -> unable to open output filename \""
        << output_filename << "\'\n\n";

   exit ( 1 );

}

table.set_size(1 + Nobj, n_header_3d_cols + n_3d_single_cols);

table.set_ics(2);

table.set_bad_data_value(bad_data_double);
table.set_bad_data_str(na_str);

   //
   //  column headings
   //

r = 0;

c = 0;

for (j=0; j<n_header_3d_cols; ++j)  {

   table.set_entry(r, c++, header_3d_cols[j]);

}

for (j=0; j<n_3d_single_cols; ++j)  {

   table.set_entry(r, c++, att_3d_single_cols[j]);

}

   //
   //  leading columns
   //

for (j=0; j<Nobj; ++j)  {

   r = j + 1;

   config.write_header_cols(table, r);

}

   //
   //  overwrite the obs valid entry in the header columns
   //

// const int  obs_valid_column = 6;   //  0-based

r = 0;

for (j=0; j<(att.n()); ++j)  {

   ++r;

   table.set_entry(r,  obs_valid_column, na_str);

}

   //
   //  attributes
   //

r = 1;

for (j=0; j<(att.n()); ++j)  {

   att[j].write_txt(table, r++);

}

   //
   //  done
   //

out << table;

out.close();

return;

}


////////////////////////////////////////////////////////////////////////


void do_3d_pair_txt_output(const PairAtt3DArray & pa,
                           const MtdConfigInfo & config,
                           const bool is_cluster,
                           const char * output_filename)

{

if ( !(config.do_3d_att_ascii) )  return;

int j, r, c;
ofstream out;
AsciiTable table;
PairAtt3DArray pa_new;
const PairAtt3DArray * a = 0;

   //
   //  if we're doing clusters, make a new array where all the object numbers match
   //

if ( is_cluster )  {

   for (j=0; j<(pa.n()); ++j)  {

      if ( pa.fcst_cluster_number(j) != pa.obs_cluster_number(j) )  continue;

      if ( pa.fcst_cluster_number(j) <= 0 )  continue;

      pa_new.add(pa[j]);

   }

   a = &pa_new;

} else {

   a = &pa;

}

const PairAtt3DArray & array = *a;

out.open(output_filename);

if ( ! out )  {

   mlog << Error << "\n\n  do_3d_pair_txt_output() -> unable to open output filename \""
        << output_filename << "\'\n\n";

   exit ( 1 );

}

table.set_size(1 + array.n(), n_header_3d_cols + n_att_3d_pair_cols);

table.set_ics(2);

table.set_bad_data_value(bad_data_double);
table.set_bad_data_str(na_str);

   //
   //  column headings
   //

r = 0;

c = 0;

for (j=0; j<n_header_3d_cols; ++j)  {

   table.set_entry(r, c++, header_3d_cols[j]);

}

for (j=0; j<n_att_3d_pair_cols; ++j)  {

   table.set_entry(r, c++, att_3d_pair_cols[j]);

}

   //
   //  leading columns
   //

for (j=0; j<(array.n()); ++j)  {

   r = j + 1;

   config.write_header_cols(table, r);

}

   //
   //  attributes
   //

r = 1;

for (j=0; j<(array.n()); ++j)  {

   array[j].write_txt(table, r++);

}

   //
   //  done
   //

out << table;

out.close();

return;

}


////////////////////////////////////////////////////////////////////////


void do_2d_txt_output(const MtdFloatFile & fcst_raw,
                      const MtdFloatFile &  obs_raw,
                      const SingleAtt2DArray & fcst_simple_att,
                      const SingleAtt2DArray &  obs_simple_att,
                      const SingleAtt2DArray & fcst_cluster_att,
                      const SingleAtt2DArray &  obs_cluster_att,
                      const MtdConfigInfo & config,
                      const char * output_filename)

{

if ( !(config.do_2d_att_ascii) )  return;

int j, r, c;
int t;
ofstream out;
AsciiTable table;
const int n_total = fcst_simple_att.n() + obs_simple_att.n() + fcst_cluster_att.n() + obs_cluster_att.n();


out.open(output_filename);

if ( ! out )  {

   mlog << Error << "\n\n  do_2d_txt_output() -> unable to open output filename \""
        << output_filename << "\'\n\n";

   exit ( 1 );

}

table.set_size(1 + n_total, n_header_3d_cols + n_2d_cols);

table.set_ics(2);

table.set_bad_data_value(bad_data_double);
table.set_bad_data_str(na_str);

   //
   //  column headings
   //

r = 0;

c = 0;

for (j=0; j<n_header_3d_cols; ++j)  {

   table.set_entry(r, c++, header_3d_cols[j]);

}

for (j=0; j<n_2d_cols; ++j)  {

   table.set_entry(r, c++, att_2d_cols[j]);

}

   //
   //  leading columns
   //

for (j=0; j<n_total; ++j)  {

   r = j + 1;

   config.write_header_cols(table, r);

}

   //
   //  overwrite the fcst and obs valid and lead entries in the header columns
   //


r = 0;

for (j=0; j<(fcst_simple_att.n()); ++j)  {

   ++r;

   t = fcst_simple_att.time_index(j);

   table.set_entry(r, fcst_lead_column, sec_to_hhmmss(fcst_simple_att.lead_time(j)));

   table.set_entry(r, fcst_valid_column, unix_to_yyyymmdd_hhmmss(fcst_simple_att.valid_time(j)));

   table.set_entry(r,  obs_valid_column, unix_to_yyyymmdd_hhmmss(obs_raw.valid_time(t)));

   table.set_entry(r,  obs_lead_column,  sec_to_hhmmss(obs_raw.lead_time(t)));

}

for (j=0; j<(obs_simple_att.n()); ++j)  {

   ++r;

   t = obs_simple_att.time_index(j);

   table.set_entry(r, fcst_valid_column, unix_to_yyyymmdd_hhmmss(fcst_raw.valid_time(t)));

   table.set_entry(r,  fcst_lead_column, sec_to_hhmmss(fcst_raw.lead_time(t)));

   table.set_entry(r,  obs_valid_column, unix_to_yyyymmdd_hhmmss(obs_simple_att.valid_time(j)));

   table.set_entry(r,  obs_lead_column, sec_to_hhmmss(obs_simple_att.lead_time(j)));

}

for (j=0; j<(fcst_cluster_att.n()); ++j)  {

   ++r;

   t = fcst_cluster_att.time_index(j);

   table.set_entry(r, fcst_lead_column, sec_to_hhmmss(fcst_cluster_att.lead_time(j)));

   table.set_entry(r, fcst_valid_column, unix_to_yyyymmdd_hhmmss(fcst_cluster_att.valid_time(j)));

   table.set_entry(r,  obs_valid_column, unix_to_yyyymmdd_hhmmss(obs_raw.valid_time(t)));

   table.set_entry(r,  obs_lead_column,  sec_to_hhmmss(obs_raw.lead_time(t)));

}

for (j=0; j<(obs_cluster_att.n()); ++j)  {

   ++r;

   t = obs_cluster_att.time_index(j);

   table.set_entry(r, fcst_valid_column, unix_to_yyyymmdd_hhmmss(fcst_raw.valid_time(t)));

   table.set_entry(r,  fcst_lead_column, sec_to_hhmmss(fcst_raw.lead_time(t)));

   table.set_entry(r,  obs_valid_column, unix_to_yyyymmdd_hhmmss(obs_cluster_att.valid_time(j)));

   table.set_entry(r,  obs_lead_column, sec_to_hhmmss(obs_cluster_att.lead_time(j)));

}

   //
   //  attributes
   //

r = 1;

for (j=0; j<(fcst_simple_att.n()); ++j)  {

   fcst_simple_att[j].write_txt(table, r++);

}

for (j=0; j<(obs_simple_att.n()); ++j)  {

   obs_simple_att[j].write_txt(table, r++);

}

for (j=0; j<(fcst_cluster_att.n()); ++j)  {

   fcst_cluster_att[j].write_txt(table, r++);

}

for (j=0; j<(obs_cluster_att.n()); ++j)  {

   obs_cluster_att[j].write_txt(table, r++);

}

   //
   //  done
   //

out << table;

out.close();

return;

}


////////////////////////////////////////////////////////////////////////

   //
   //  for single fields
   //

void do_2d_txt_output(const MtdFloatFile & fcst_raw,
                      const MtdFloatFile &  obs_raw,
                      const SingleAtt2DArray & att,
                      const MtdConfigInfo & config,
                      const char * output_filename)

{

if ( !(config.do_2d_att_ascii) )  return;

int j, r, c;
ofstream out;
AsciiTable table;
const int n_total = att.n();


out.open(output_filename);

if ( ! out )  {

   mlog << Error << "\n\n  do_2d_txt_output[single]() -> unable to open output filename \""
        << output_filename << "\'\n\n";

   exit ( 1 );

}

table.set_size(1 + n_total, n_header_3d_cols + n_2d_cols);

table.set_ics(2);

table.set_bad_data_value(bad_data_double);
table.set_bad_data_str(na_str);

   //
   //  column headings
   //

r = 0;

c = 0;

for (j=0; j<n_header_3d_cols; ++j)  {

   table.set_entry(r, c++, header_3d_cols[j]);

}

for (j=0; j<n_2d_cols; ++j)  {

   table.set_entry(r, c++, att_2d_cols[j]);

}

   //
   //  leading columns
   //

for (j=0; j<n_total; ++j)  {

   r = j + 1;

   config.write_header_cols(table, r);

}

   //
   //  overwrite the fcst and obs valid and lead entries in the header columns
   //

const int fcst_valid_column = 4;   //  0-based
const int  obs_valid_column = 6;   //  0-based

r = 0;

for (j=0; j<(att.n()); ++j)  {

   ++r;

   table.set_entry(r, fcst_valid_column - 1, sec_to_hhmmss(att.lead_time(j)));
   table.set_entry(r,  obs_valid_column - 1, sec_to_hhmmss(att.lead_time(j)));

   table.set_entry(r, fcst_valid_column, unix_to_yyyymmdd_hhmmss(att.valid_time(j)));
   table.set_entry(r,  obs_valid_column, unix_to_yyyymmdd_hhmmss(att.valid_time(j)));

}

   //
   //  attributes
   //

r = 1;

for (j=0; j<(att.n()); ++j)  {

   att[j].write_txt(table, r++);

}


   //
   //  done
   //

out << table;

out.close();

return;

}


////////////////////////////////////////////////////////////////////////


