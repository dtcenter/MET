

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


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static const int buf_size = 1 << 17;

static unsigned char buf[buf_size];

static char date_string[256];

static char variable [4];


////////////////////////////////////////////////////////////////////////


   //
   //  zero-based
   //

inline int fortran_two_to_one(const int N1, const int v1, const int v2) { return ( v2*N1 + v1 ); }


////////////////////////////////////////////////////////////////////////


static void usage();

static bool fortran_read(int fd, void *, int n_bytes);

static double rdiag_get_2d(void *, const int nreal, int i, int j);   //  1-based

static double rdiag_get_guess          (void *, const int nreal, int j);   //  1-based
// static double rdiag_get_guess_no_bias  (void *, const int nreal, int j);   //  1-based

static void get_cdiag(int index, const void * cdiag_buf, char * out);   //  1-based

static void do_row(AsciiTable & table, int row, const char * id, void * b, const int nreal, const int j);


////////////////////////////////////////////////////////////////////////


int main(int argc, char * argv [])

{

program_name = get_short_name(argv[0]);

if ( argc != 3 )  usage();

const ConcatString  input_filename = argv[1];
const ConcatString output_filename = argv[2];

   //
   //  open files
   //

int j, k;
int row;
int in = -1;
int date, n_bytes;
int nchar, nreal, ii, mtype;
int n_rdiag;
int cdiag_bytes, rdiag_bytes;
int year, month, day, hour;
char id[9];
ofstream out;
AsciiTable table;
StatHdrColumns columns;
float * f = (float *) 0;


table.set_size(1, 32);   //  for MPR line types

for (j=0; j<n_header_columns; ++j)  table.set_entry(0, j, hdr_columns[j]);

for (j=0; j<n_mpr_columns; ++j)  {

   k = n_header_columns + j;

   table.set_entry(0, k, mpr_columns[j]);

}

if ( (in = open(input_filename, O_RDONLY)) < 0 )  {

   cerr << "\n\n  " << program_name << ": unable to open input file \""
        << input_filename << "\n\n";

   exit ( 1 );

}

out.open(output_filename);

if ( ! out )  {

   cerr << "\n\n  " << program_name << ": unable to open output file \""
        << output_filename << "\n\n";

   exit ( 1 );

}

   //
   //  read date
   //

fortran_read(in, &date, 4);

if ( swap_endian )  shuffle_4(&date);

// cout << "\n\n  " << date << "\n\n";

year  = date/1000000;
month = (date/10000)%100;
day   = (date/100)%100;
hour  = date%100;

sprintf(date_string, "%04d%02d%02d_%02d0000", year, month, day, hour);


   //
   //  read data
   //

row = 1;

while ( fortran_read(in, buf, 19) )  {

   memcpy(variable, buf, 3);

   variable[3] = (char) 0;

   memcpy(&nchar, buf +  3, 4);
   memcpy(&nreal, buf +  7, 4);
   memcpy(&ii,    buf + 11, 4);
   memcpy(&mtype, buf + 15, 4);

   if ( swap_endian )  {

      shuffle_4(&nchar);
      shuffle_4(&nreal);
      shuffle_4(&ii);
      shuffle_4(&mtype);

   }

   // cout << "\n  ii    = " << ii    << "\n";
   // cout << "\n  nreal = " << nreal << "\n";

   // cout << "\n  var   = \"" << var << "\"\n";

      //
      //  read data
      //

   n_rdiag = nreal*ii;

   cdiag_bytes = 8*ii;
   rdiag_bytes = 4*n_rdiag;

   n_bytes = cdiag_bytes + rdiag_bytes;

   if ( n_bytes >= buf_size )  {

      cerr << "\n\n  " << program_name << ": buffer too small ... increase to at least "
           << n_bytes << " bytes!\n\n";

      exit ( 1 );

   }


   // cout << "\n\n  n_bytes = " << n_bytes << "\n\n";

   fortran_read(in, buf, n_bytes);

   f = (float *) (buf + cdiag_bytes);

   if ( swap_endian )  { for (j=0; j<n_rdiag; ++j)  shuffle_4(f + j); }

   table.add_rows(ii);


   for (j=1; j<=ii; ++j)  {

      get_cdiag(j, buf, id);

      do_row(table, row++, id, f, nreal, j);

   }

}   //  while

out << table;

    //
    //  done
    //

close(in);  in = -1;

out.close();

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " infile outfile\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


bool fortran_read(int fd, void * b, int n_bytes)

{

long long n_read;

n_read = read_fortran_binary(fd, b, n_bytes, rec_pad_length, swap_endian);

if ( n_read == 0 )  return ( false );

if ( (n_read < 0) || (n_read != n_bytes) )  {

   cerr << "\n\n  " << program_name << ": fortran_read() -> read error!\n\n";

   exit ( 1 );

}

return ( true );

}


////////////////////////////////////////////////////////////////////////


double rdiag_get_2d(void * b, const int nreal, int i, int j)   //  1-based

{

float * f = (float *) b;
int n;
double value;


n = fortran_two_to_one(nreal, i - 1, j - 1);

// shuffle_4(f + n);

value = (double) (f[n]);


return ( value );

}


////////////////////////////////////////////////////////////////////////


double rdiag_get_guess(void * b, const int nreal, int j)   //  1-based

{

double obs, obs_minus_guess;

obs             = rdiag_get_2d(b, nreal, obs_data_index, j);

obs_minus_guess = rdiag_get_2d(b, nreal, omg_index,      j);


return ( obs - obs_minus_guess );

}


////////////////////////////////////////////////////////////////////////

/*
double rdiag_get_guess_no_bias(void * b, const int nreal, int j)   //  1-based

{

double obs, obs_minus_guess;

obs             = rdiag_get_2d(b, nreal, obs_data_index,    j);

obs_minus_guess = rdiag_get_2d(b, nreal, omg_no_bias_index, j);


return ( obs - obs_minus_guess );

}
*/

////////////////////////////////////////////////////////////////////////


void get_cdiag(int index, const void * cdiag_buf, char * out)   //  1-based

{

const char * s = (const char *) cdiag_buf;

memcpy(out, s + 8*(index - 1), 8);

out[8] = (char) 0;

int j;

for (j=7; j>=0; --j)  {

   if ( out[j] == ' ' )  out[j] = (char) 0;

   else break;

}


   //
   //  done
   //

return;

}

////////////////////////////////////////////////////////////////////////


void do_row(AsciiTable & table, int row, const char * id, void * b, const int nreal, int j)

{

int col = 0;
char junk[256];
const char * model = "XXX";

const double lat       = rdiag_get_2d(b, nreal,       lat_index, j);
const double lon       = rdiag_get_2d(b, nreal,       lon_index, j);
const double pressure  = rdiag_get_2d(b, nreal,  pressure_index, j);
const double obs_value = rdiag_get_2d(b, nreal,  obs_data_index, j);
const double elevation = rdiag_get_2d(b, nreal, elevation_index, j);

const double guess         = rdiag_get_guess         (b, nreal, j);
// const double guess_no_bias = rdiag_get_guess_no_bias (b, nreal, j);

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
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


