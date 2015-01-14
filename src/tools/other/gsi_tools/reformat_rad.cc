

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
#include "rad_offsets.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Default values for command-line switches
   //

static int channel = -1;   //  zero based


////////////////////////////////////////////////////////////////////////


static ConcatString program_name;

static CommandLine cline;

static const int buf_size = 1 << 14;

static unsigned char buf[buf_size];


////////////////////////////////////////////////////////////////////////


   //
   //  zero-based
   //

inline int fortran_two_to_one(const int N1, const int v1, const int v2) { return ( v2*N1 + v1 ); }


////////////////////////////////////////////////////////////////////////


struct RadParams {

   char obstype [11]; 
   char dplat   [11]; 
   char isis    [21]; 

   int jiter;
   int nchanl;
   int npred;
   int idate;
   int ireal;
   int ipchan;
   int iextra;
   int jextra;

   RadParams();

};

struct ChannelParams {

   float freq;
   float plo;
   float wave;
   float varch;
   float tlap;

   int iuse;
   int nuchan;
   int ich;

   ChannelParams();

};


////////////////////////////////////////////////////////////////////////


static void usage();

static void set_channel(const StringArray &);

static void my_memcpy(void * to, unsigned char * & from, int);

static void read_params(int fd, RadParams &);

static void read_channel(int fd, ChannelParams &);

static bool read_data(int fd, const int n_diag, const int N1, const int N2, float * diag, float * diagchan);

static void do_row(AsciiTable &, const int row, const unixtime t0, const int N1, const int N2, const float * diag, const float * diagchan);


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
int in = -1;
int count;
int row;
ofstream out;
AsciiTable table;
StatHdrColumns columns;
RadParams params;
unixtime t0;
int month, day, year, hour;
ChannelParams * cp = 0;
bool status = false;
float * diag     = 0;
float * diagchan = 0;


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
   //  read parameters
   //

read_params(in, params);

if ( channel >= params.nchanl )  {

   cerr << "\n\n  " << program_name << ": bad channel selected ... there are only "
        << params.nchanl << " channels\n\n";

   exit ( 1 );

}

k = params.idate;   //  YYYYMMDDHH

month = (k/10000)%100;
day   = (k/100)%100;
year  = k/1000000;
hour  = k%100;

t0 = mdyhms_to_unix(month, day, year, hour, 0, 0);

cp = new ChannelParams [params.nchanl];

for (j=0; j<(params.nchanl); ++j)  {

   read_channel(in, cp[j]);

}

   //
   //  read data
   //

const int n_diag = params.ireal;

diag = new float [n_diag];

const int N1 = params.ipchan + params.npred + 1;
const int N2 = params.nchanl;

diagchan = new float [N1*N2];

// cout << "(N1, N2) = (" << N1 << ", " << N2 << ")\n";

// cout << "diag = " << n_diag << " floats\n";
// 
// cout << "diagchan = " << (N1*N2) << " floats\n";

count = 0;

row = 1;   //  allow for header line

while ( 1 )  {

   status = read_data(in, n_diag, N1, N2, diag, diagchan);

   if ( ! status )  break;

   table.add_rows(1);

   do_row(table, row++, t0, N1, N2, diag, diagchan);

   ++count;

}   //  while


cout << "\n\n  Read " << count << " data records\n\n";


    //
    //  done
    //

out << table;

close(in);  in = -1;

out.close();

if ( cp )  { delete [] cp;  cp = 0; }

if ( diag )  { delete [] diag;  diag = 0; }

if ( diagchan )  { delete [] diagchan;  diagchan = 0; }

return ( 0 );

}


////////////////////////////////////////////////////////////////////////


void usage()

{

cerr << "\n\n   usage:  " << program_name << " -channel n infile outfile\n\n"
     << "          (Note: channel numbers are 1-based)\n\n";

exit ( 1 );

return;

}


////////////////////////////////////////////////////////////////////////


void set_channel(const StringArray & a)

{

int k = atoi(a[0]);

if ( k <= 0 )  {

   cerr << "\n\n  " << program_name << ": bad channel number ... " << k << "\n\n";

   exit ( 1 );

}

channel = k - 1;

return;

}


////////////////////////////////////////////////////////////////////////


RadParams::RadParams()

{

memset(obstype, 0, sizeof(obstype));
memset(dplat,   0, sizeof(dplat)); 
memset(isis,    0, sizeof(isis)); 

jiter  = 0;
nchanl = 0;
npred  = 0;
idate  = 0;
ireal  = 0;
ipchan = 0;
iextra = 0;
jextra = 0;

}


////////////////////////////////////////////////////////////////////////


ChannelParams::ChannelParams()

{

freq  = 0.0;
plo   = 0.0;
wave  = 0.0;
varch = 0.0;
tlap  = 0.0;

iuse   = 0;
nuchan = 0;
ich    = 0;

}


////////////////////////////////////////////////////////////////////////


void my_memcpy(void * to, unsigned char * & from, int n_bytes)

{

memcpy(to, from, n_bytes);

from += n_bytes;


return;

}


////////////////////////////////////////////////////////////////////////


void read_params(int fd, RadParams & params)

{

long long n_read;

n_read = read_fortran_binary(fd, buf, buf_size, rec_pad_length, swap_endian);

if ( n_read < 72 )  {

   cerr << "\n\n  " << program_name << ": failed to read params\n\n";

   exit ( 1 );

}

unsigned char * b = buf;

my_memcpy( &(params.isis),    b, 20);
my_memcpy( &(params.dplat),   b, 10);
my_memcpy( &(params.obstype), b, 10);

my_memcpy( &(params.jiter),   b, 4);
my_memcpy( &(params.nchanl),  b, 4);
my_memcpy( &(params.npred),   b, 4);
my_memcpy( &(params.idate),   b, 4);
my_memcpy( &(params.ireal),   b, 4);
my_memcpy( &(params.ipchan),  b, 4);
my_memcpy( &(params.iextra),  b, 4);
my_memcpy( &(params.jextra),  b, 4);

if ( swap_endian )  {

   shuffle_4( &(params.jiter)  );
   shuffle_4( &(params.nchanl) );
   shuffle_4( &(params.npred)  );
   shuffle_4( &(params.idate)  );
   shuffle_4( &(params.ireal)  );
   shuffle_4( &(params.ipchan) );
   shuffle_4( &(params.iextra) );
   shuffle_4( &(params.jextra) );

}



   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void read_channel(int fd, ChannelParams & cp)

{

long long n_read;

n_read = read_fortran_binary(fd, buf, buf_size, rec_pad_length, swap_endian);

if ( n_read != 32 )  {

   cerr << "\n\n  " << program_name << ": failed to read channel info\n\n";

   exit ( 1 );

}

unsigned char * b = buf;

my_memcpy( &(cp.freq),   b, 4);
my_memcpy( &(cp.plo),    b, 4);
my_memcpy( &(cp.wave),   b, 4);
my_memcpy( &(cp.varch),  b, 4);
my_memcpy( &(cp.tlap),   b, 4);
my_memcpy( &(cp.iuse),   b, 4);
my_memcpy( &(cp.nuchan), b, 4);
my_memcpy( &(cp.ich),    b, 4);

if ( swap_endian )  {

   shuffle_4( &(cp.freq)   );
   shuffle_4( &(cp.plo)    );
   shuffle_4( &(cp.wave)   );
   shuffle_4( &(cp.varch)  );
   shuffle_4( &(cp.tlap)   );
   shuffle_4( &(cp.iuse)   );
   shuffle_4( &(cp.nuchan) );
   shuffle_4( &(cp.ich)    );

}





   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool read_data(int fd, const int n_diag, const int N1, const int N2, float * diag, float * diagchan)

{

const int bytes = (int) ((n_diag + N1*N2)*sizeof(float));
int n_read;
const int n12 = N1*N2;

if ( bytes > buf_size )  {

   cerr << "\n\n  " << program_name << ": read_data() -> buffer too small "
        << "... increase size to at least " << bytes << " bytes\n\n";

   exit ( 1 );

}

n_read = read_fortran_binary(fd, buf, buf_size, rec_pad_length, swap_endian);

if ( n_read == 0 )  return ( false );

if ( n_read < bytes )  {

   cerr << "\n\n  " << program_name << ": read_data() -> read error\n\n";

   exit ( 1 );

}

unsigned char * b = buf;

my_memcpy(diag,     b, 4*n_diag);
my_memcpy(diagchan, b, 4*n12);

if ( swap_endian )  {

   int j;

   for (j=0; j<n_diag; ++j)  shuffle_4(diag + j);

   for (j=0; j<n12; ++j)  shuffle_4(diagchan + j);

}


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


void do_row(AsciiTable & table, const int row, const unixtime t0, const int N1, const int N2, const float * diag, const float * diagchan)

{

int k;
int month, day, year, hour, minute, second;
int col = 0;
char junk[256];
const char * model = "XXX";
char date_string[256];
char variable[256];

const char * const       id  = "id";

const double lat             = diag[lat_index - 1];
const double lon             = diag[lon_index - 1];
const double elev            = diag[elevation_index - 1];
const double obs_time        = diag[dtime_index - 1];

const double obs_value       = diagchan[fortran_two_to_one(N1,  btemp_chan_index - 1, channel)];
const double obs_minus_guess = diagchan[fortran_two_to_one(N1, omg_bc_chan_index - 1, channel)];
const double guess           = obs_value - obs_minus_guess;


// cout << "Obs = "             << obs_value          << '\n';
// cout << "Elev = "            << elev               << '\n';
// cout << "Obs time = "        << obs_time           << '\n';
// cout << "Obs minus guess = " << obs_minus_guess    << '\n';
// cout << "(Lat, Lon) = ("     << lat << ", " << lon << ")\n";


snprintf(variable, sizeof(variable), "TB_%02d", channel + 1);


k = nint(3600*obs_time);

unix_to_mdyhms(t0 + k, month, day, year, hour, minute, second);

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
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////




