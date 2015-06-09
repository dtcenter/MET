// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////
//
//   Filename:   reformat_gsi.cc
//
//   Description:
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    6/9/15    Bullock         New
//
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <cstdio>
#include <cmath>

#include "vx_util.h"
#include "vx_math.h"
#include "vx_stat_out.h"
#include "config_constants.h"
#include "vx_stat_out.h"
#include "vx_log.h"

#include "read_fortran_binary.h"
#include "conv_offsets.h"
#include "conv_record.h"

////////////////////////////////////////////////////////////////////////
//
// Constants
//
////////////////////////////////////////////////////////////////////////

static const char  *program_name = "reformat_gsi";
static const int  rec_pad_length =    4;
static const bool    swap_endian = true;

////////////////////////////////////////////////////////////////////////
//
// Variables for Command Line Arguments
//
////////////////////////////////////////////////////////////////////////

static ConcatString output_directory = ".";
static NumArray channel;

////////////////////////////////////////////////////////////////////////

// JHG, move this section to a header file

static const char * const conv_extra_columns [] = {

   "OBS_STYPE",    //  observation subtype                 (1)
   "OBERR_IN",     //  prepbufr inverse observation error  (14)
   "FIN_INV_ERR",  //  final inverse observation error     (16)

   "OBS_HGT",      //  observation height                  (7)
   "OBS_TIME",     //  observation time                    (8)
   "INPUT_QC",     //  input prepbufr qc                   (9)

   "SETUP_QC",     //  setup qc                            (10)
   "PREP_USE",     //  read_prepbufr usage                 (11)
   "ANLY_USE",     //  analysis usage                      (12)

   "RWGT",         //  non-linear qc rel weight            (13)
   "OBERR_ADJ",    //  read_prepbufr inverse obs error     (15)

};

static const int n_extra_cols = sizeof(conv_extra_columns)/sizeof(*conv_extra_columns);

////////////////////////////////////////////////////////////////////////

static void process(const char * conv_filename);
static void do_row(AsciiTable & table, int row, ConvRecord & r, const int j);

static void usage();
static void set_channel(const StringArray &);
static void set_outdir(const StringArray &);
static void set_logfile(const StringArray &);
static void set_verbosity(const StringArray &);

////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv []) {
   CommandLine cline;
   
   // Parse the command line into tokens
   cline.set(argc, argv);
   
   // Set the usage
   cline.set_usage(usage);

   // Add options
   cline.add(set_channel,   "-channel", 1);
   cline.add(set_outdir,    "-outdir",  1);
   cline.add(set_logfile,   "-log",     1);
   cline.add(set_verbosity, "-v",       1);

   // Parse the command line
   cline.parse();

   // Check for zero files to process
   if(cline.n() == 0) usage();

   // Process each remaining argument
   for (int i=0; i<(cline.n()); i++) {

      mlog << Debug(1)
           << "Reading \"" << cline[i] << "\" ... " << (i + 1)
           << " of " << cline.n() << "\n";

      if((i%5) == 4) mlog << Debug(1) << "\n";

      process(cline[i]);
   }

   return ( 0 );
}

////////////////////////////////////////////////////////////////////////


void process(const char * conv_filename)

{

   //
   //  open files
   //

int j, k;
int row;
int date, n_bytes;
int nchar, nreal, ii, mtype;
int n_rdiag;
int cdiag_bytes, rdiag_bytes;
int year, month, day, hour;
ofstream out;
AsciiTable table;
StatHdrColumns columns;
long long size = 0;
ConcatString output_filename;
ConvFile f;
ConvRecord r;


output_filename << cs_erase
                << output_directory << '/'
                << get_short_name(conv_filename) << ".mpr";


table.set_size(1, n_header_columns + n_mpr_columns + n_extra_cols);   //   added 3 extra columns

for (j=0; j<n_header_columns; ++j)  table.set_entry(0, j, hdr_columns[j]);

for (j=0; j<n_mpr_columns; ++j)  {

   k = n_header_columns + j;

   table.set_entry(0, k, mpr_columns[j]);

}

for (j=0; j<n_extra_cols; ++j)  {

   k = n_header_columns + n_mpr_columns + j;

   table.set_entry(0, k, conv_extra_columns[j]);

}

if ( !(f.open(conv_filename)) )  {

   mlog << Error << "\n\n  " << program_name << ": unable to open input file \""
        << conv_filename << "\n\n";

   exit ( 1 );

}

mlog << Debug(1)
     << "Writing \"" << output_filename << "\"\n";

out.open(output_filename);

if ( ! out )  {

   mlog << Error << "\n\n  " << program_name << ": unable to open output file \""
        << output_filename << "\n\n";

   exit ( 1 );

}

   //
   //  read data
   //

row = 1;

while ( (f >> r) )  {

   table.add_rows(r.ii);

   for (j=0; j<(r.ii); ++j)  {

      do_row(table, row++, r, j);

   }

}   //  while

out << table;

    //
    //  done
    //

f.close();

out.close();

return;

}

////////////////////////////////////////////////////////////////////////


void do_row(AsciiTable & table, int row, ConvRecord & r, const int j)

{

int col = 0;
char junk[256];
const char * model = "XXX";
const ConcatString date_string = r.date_string();
const ConcatString id          = r.station_name(j);

const double lat       = r.rdiag_get_2d(      lat_index - 1, j);
const double lon       = r.rdiag_get_2d(      lon_index - 1, j);
const double pressure  = r.rdiag_get_2d( pressure_index - 1, j);
const double obs_value = r.rdiag_get_2d( obs_data_index - 1, j);
const double elevation = r.rdiag_get_2d(elevation_index - 1, j);

const double obs_subtype     = r.rdiag_get_2d (  obssubtype_index - 1, j);
const double pb_inv_error    = r.rdiag_get_2d(   pb_inverse_index - 1, j);
const double final_inv_error = r.rdiag_get_2d(final_inverse_index - 1, j);

const double guess         = r.rdiag_get_guess (j);

const double obs_hgt       = r.rdiag_get_2d(         height_index - 1, j);
const double obs_time      = r.rdiag_get_2d(      obs_hours_index - 1, j);
const double input_qc      = r.rdiag_get_2d(       input_qc_index - 1, j);

const double setup_qc      = r.rdiag_get_2d(       setup_qc_index - 1, j);
const double prep_use      = r.rdiag_get_2d(          usage_index - 1, j);
const double analy_use     = r.rdiag_get_2d(   analysis_use_index - 1, j);

const double rwgt          = r.rdiag_get_2d(      qc_weight_index - 1, j);
const double oberr_adj     = r.rdiag_get_2d(read_pb_inverse_index - 1, j);

   //
   //  first 21 columns
   //

table.set_entry(row, col++, met_version);   //  version

table.set_entry(row, col++, model);         //  model

table.set_entry(row, col++, "000000");      //  fcst lead

table.set_entry(row, col++, date_string);   //  fcst valid begin
table.set_entry(row, col++, date_string);   //  fcst valid end

table.set_entry(row, col++, "000000");      //  obs lead

table.set_entry(row, col++, date_string);   //  obs valid begin
table.set_entry(row, col++, date_string);   //  obs valid end

table.set_entry(row, col++, r.variable);    //  fcst var
table.set_entry(row, col++, stat_na_str);   //  fcst level

table.set_entry(row, col++, r.variable);    //  obs var
table.set_entry(row, col++, stat_na_str);   //  obs level

table.set_entry(row, col++, stat_na_str);   //  obtype

table.set_entry(row, col++, stat_na_str);   //  mask

table.set_entry(row, col++, stat_na_str);   //  interp method

table.set_entry(row, col++, 0);             //  interp points

table.set_entry(row, col++, stat_na_str);   //  fcst thresh
table.set_entry(row, col++, stat_na_str);   //  obs  thresh
table.set_entry(row, col++, stat_na_str);   //  cov  thresh

table.set_entry(row, col++, stat_na_str);   //  alpha

table.set_entry(row, col++, stat_mpr_str);  //  line type


   //
   //  MPR-specific columns
   //


table.set_entry(row, col++, 1);             //  total

table.set_entry(row, col++, 0);             //  index

table.set_entry(row, col++, id);            //  station id

sprintf(junk, "%.4f", lat);
table.set_entry(row, col++, junk);          //  latitude

sprintf(junk, "%.4f", lon);
table.set_entry(row, col++, junk);          //  longitude

sprintf(junk, "%.1f", pressure);
table.set_entry(row, col++, junk);          //  pressure (obs_lvl)

sprintf(junk, "%.1f", elevation);
table.set_entry(row, col++, junk);          //  elevation

sprintf(junk, "%.4f", guess);
table.set_entry(row, col++, junk);          //  fcst value

sprintf(junk, "%.4f", obs_value);
table.set_entry(row, col++, junk);          //  obs value

table.set_entry(row, col++, stat_na_str);   //  climatological value

table.set_entry(row, col++, stat_na_str);   //  qc value


   //
   //  extra columns
   //

table.set_entry(row, col++, obs_subtype);
table.set_entry(row, col++, pb_inv_error);
table.set_entry(row, col++, final_inv_error);

table.set_entry(row, col++, obs_hgt);
table.set_entry(row, col++, obs_time);
table.set_entry(row, col++, input_qc);

table.set_entry(row, col++, setup_qc);
table.set_entry(row, col++, prep_use);
table.set_entry(row, col++, analy_use);

table.set_entry(row, col++, rwgt);
table.set_entry(row, col++, oberr_adj);

   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////

void usage() {

   cout << "\n*** Model Evaluation Tools (MET" << met_version
        << ") ***\n\n"

        << "Usage: " << program_name << "\n"
        << "\tgsi_file1 [gsi_file2 gsi_file3 ... gsi_filen]\n"
        << "\t[-channel n]\n"
        << "\t[-outdir path]\n"
        << "\t[-log file]\n"
        << "\t[-v level]\n\n"

        << "\twhere\t\"gsi_file\" is a GSI binary file (conventional or "
        << "radiance) to be reformatted (required).\n"
        << "\t\t\"-channel n\" overrides the default processing of all "
        << "radiance channels with a comma-separated list (optional).\n"
        << "\t\t\"-outdir path\" overrides the default output directory ("
        << output_directory << ") (optional).\n"
        << "\t\t\"-log file\" outputs log messages to the specified "
        << "file (optional).\n"
        << "\t\t\"-v level\" overrides the default level of logging ("
        << mlog.verbosity_level() << ") (optional).\n\n"
        << flush;

   exit (1);
}

////////////////////////////////////////////////////////////////////////

void set_channel(const StringArray & a) {
   channel.add_css(a[0]);
}

////////////////////////////////////////////////////////////////////////

void set_outdir(const StringArray & a) {
   output_directory = a[0];
}

////////////////////////////////////////////////////////////////////////

void set_logfile(const StringArray & a) {
   ConcatString filename;

   filename = a[0];

   mlog.open_log_file(filename);
}

////////////////////////////////////////////////////////////////////////

void set_verbosity(const StringArray & a) {
   mlog.set_verbosity_level(atoi(a[0]));
}

////////////////////////////////////////////////////////////////////////
