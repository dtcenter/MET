

////////////////////////////////////////////////////////////////////////


static const int  rec_pad_length =    4;
static const bool    swap_endian = true;


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

#include "read_fortran_binary.h"
#include "conv_offsets.h"
#include "conv_record.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //

static ConcatString output_directory = ".";


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static const char * const conv_extra_columns [] = {

   "OBS_STYPE",    //  observation subtype
   "PB_INV_ERR",   //  prepbufr inverse observation error
   "FIN_INV_ERR",  //  final inverse observation error

};

static const int n_extra_cols = sizeof(conv_extra_columns)/sizeof(*conv_extra_columns);


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_outdir(const StringArray &);

static void process(const char * conv_filename);

static void do_row(AsciiTable & table, int row, ConvRecord & r, const int j);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_outdir, "-outdir", 1);

cline.parse();


if ( cline.n() == 0  )  usage();

int j;

for (j=0; j<(cline.n()); ++j)  {

   cout << "Processing \"" << cline[j] << "\" ... " << (j + 1) << " of " << cline.n() << '\n';

   if ( (j%5) == 4 )  cout.put('\n');

   cout.flush();

   process(cline[j]);

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


void usage()

{

mlog << Error << "\n\n   usage:  " << program_name << " [ -outdir path ] conv_file_list\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_outdir(const StringArray & a)

{

output_directory = a[0];

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

// cout << pressure << '\n';

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

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


