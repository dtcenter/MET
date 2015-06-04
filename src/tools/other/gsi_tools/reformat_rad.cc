

////////////////////////////////////////////////////////////////////////


static const bool verbose = false;


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
#include "rad_offsets.h"
#include "rad_record.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //

static int channel = -1;   //  zero based


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static const char * const rad_extra_columns [] = {

   "INV_OBS_ERR",   //  inverse observation error
   "SURF_EMIS",     //  surface emissivity

};

static const int n_extra_cols = sizeof(rad_extra_columns)/sizeof(*rad_extra_columns);


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_channel(const StringArray &);

static void do_row(AsciiTable &, const int row, const RadRecord &);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

cline.set(argc, argv);

cline.set_usage(usage);

cline.add(set_channel, "-channel", 1);

cline.parse();

if ( cline.n() != 2 )  usage();

if ( channel < 0 )  usage();

const ConcatString  input_filename = cline[0];
const ConcatString output_filename = cline[1];

   //
   //  open files
   //

int j, k;
int count;
int row;
ofstream out;
AsciiTable table;
StatHdrColumns columns;
int month, day, year, hour;
bool status = false;
RadFile f;
RadRecord r;


table.set_size(1, n_header_columns + n_mpr_columns + n_extra_cols);

for (j=0; j<n_header_columns; ++j)  table.set_entry(0, j, hdr_columns[j]);

for (j=0; j<n_mpr_columns; ++j)  {

   k = n_header_columns + j;

   table.set_entry(0, k, mpr_columns[j]);

}

for (j=0; j<n_extra_cols; ++j)  {

   k = n_header_columns + n_mpr_columns + j;

   table.set_entry(0, k, rad_extra_columns[j]);

}

if ( !(f.open(input_filename)) < 0 )  {

   mlog << Error << "\n\n  " << program_name << ": unable to open input file \""
        << input_filename << "\n\n";

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

count = 0;

row = 1;   //  allow for header line

while ( (f >> r) )  {

   table.add_rows(1);

   do_row(table, row++, r);

   ++count;

}   //  while


cout << "\n\n  Read " << count << " data records\n\n";


    //
    //  done
    //

out << table;

f.close();

out.close();

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

mlog << Error << "\n\n   usage:  " << program_name << " -channel n infile outfile\n\n"
     << "          (Note: channel numbers are 1-based)\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_channel(const StringArray & a)

{

int k = atoi(a[0]);

if ( k <= 0 )  {

   mlog << Error << "\n\n  " << program_name << ": bad channel number ... " << k << "\n\n";

   exit ( 1 );

}

channel = k - 1;

return;

}


////////////////////////////////////////////////////////////////////////


void do_row(AsciiTable & table, const int row, const RadRecord & r)

{

int k;
int month, day, year, hour, minute, second;
int col = 0;
char junk[256];
const char * model = "XXX";
char date_string[256];
char variable[256];

const char * const       id  = "id";

const double lat             = r.diag_data(lat_index - 1);
const double lon             = r.diag_data(lon_index - 1);
const double elev            = r.diag_data(elevation_index - 1);
const double obs_time        = r.diag_data(dtime_index - 1);

const double obs_value       = r.diagchan_data( btemp_chan_index - 1, channel);
const double obs_minus_guess = r.diagchan_data(omg_bc_chan_index - 1, channel);

const double inv_obs_error   = r.diagchan_data(inv_chan_index - 1, channel);
const double surf_em         = r.diagchan_data(surf_em_index  - 1, channel);

const double guess           = obs_value - obs_minus_guess;


// cout << "Obs = "             << obs_value          << '\n';
// cout << "Elev = "            << elev               << '\n';
// cout << "Obs time = "        << obs_time           << '\n';
// cout << "Obs minus guess = " << obs_minus_guess    << '\n';
// cout << "(Lat, Lon) = ("     << lat << ", " << lon << ")\n";


snprintf(variable, sizeof(variable), "TB_%02d", channel + 1);


k = nint(3600*obs_time);

unix_to_mdyhms(r.date() + k, month, day, year, hour, minute, second);

snprintf(date_string, sizeof(date_string), "%04d%02d%02d_%02d%02d%02d", year, month, day, hour, minute, second);

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

table.set_entry(row, col++, variable);      //  fcst var
table.set_entry(row, col++, stat_na_str);   //  fcst level

table.set_entry(row, col++, variable);      //  obs var
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

snprintf(junk, sizeof(junk), "%.4f", lat);
table.set_entry(row, col++, junk);          //  latitude

snprintf(junk, sizeof(junk), "%.4f", lon);
table.set_entry(row, col++, junk);          //  longitude

// snprintf(junk, sizeof(junk), "%.1f", pressure);
// table.set_entry(row, col++, junk);          //  pressure (obs_lvl)
table.set_entry(row, col++, stat_na_str);

snprintf(junk, sizeof(junk), "%.1f", elev);
table.set_entry(row, col++, junk);          //  elevation

snprintf(junk, sizeof(junk), "%.4f", guess);
table.set_entry(row, col++, junk);          //  fcst value

snprintf(junk, sizeof(junk), "%.4f", obs_value);
table.set_entry(row, col++, junk);          //  obs value

table.set_entry(row, col++, stat_na_str);   //  climatological value

table.set_entry(row, col++, stat_na_str);   //  qc value

   //
   //  extra columns
   //

table.set_entry(row, col++, inv_obs_error);
table.set_entry(row, col++, surf_em);

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////




