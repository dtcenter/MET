

////////////////////////////////////////////////////////////////////////


static const int  rec_pad_length =    4;
static const bool    swap_endian = true;

static const bool verbose        = false;


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

static int buf_size = 512;

static unsigned char * buf = 0;

static const char * const rad_extra_columns [] = {

   "INV_OBS_ERR",   //  inverse observation error
   "SURF_EMIS",     //  surface emissivity

};

static const int n_extra_cols = sizeof(rad_extra_columns)/sizeof(*rad_extra_columns);


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

   int idiag;
   int angord;
   int iversion;
   int inewpc;

   RadParams();

   void dump(ostream &) const;

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

   void dump(ostream &) const;

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

buf = new unsigned char [buf_size];

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


table.set_size(1, n_header_columns + n_mpr_columns + n_extra_cols);

for (j=0; j<n_header_columns; ++j)  table.set_entry(0, j, hdr_columns[j]);

for (j=0; j<n_mpr_columns; ++j)  {

   k = n_header_columns + j;

   table.set_entry(0, k, mpr_columns[j]);

}

for (j=0; j<n_mpr_columns; ++j)  {

   k = n_header_columns + n_mpr_columns + j;

   table.set_entry(0, k, rad_extra_columns[j]);

}

if ( (in = open(input_filename, O_RDONLY)) < 0 )  {

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
   //  read parameters
   //

read_params(in, params);

if ( channel >= params.nchanl )  {

   mlog << Error << "\n\n  " << program_name << ": bad channel selected ... there are only "
        << params.nchanl << " channels\n\n";

   exit ( 1 );

}

if ( verbose )  {

   cout << "\nParams:\n";

   params.dump(cout);

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

// const int N1 = params.ipchan + params.npred + 1;
const int N1 = params.idiag;
const int N2 = params.nchanl;

diagchan = new float [N1*N2];

// cout << '\n';
// 
// cout << "(N1, N2) = (" << N1 << ", " << N2 << ")\n";
// 
// cout << "diag     = " << n_diag  << " floats\n" << flush;
// cout << "diagchan = " << (N1*N2) << " floats\n" << flush;
// 
// cout << '\n';

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

idiag    = 0;
angord   = 0;
iversion = 0;
inewpc   = 0;

}


////////////////////////////////////////////////////////////////////////


void RadParams::dump(ostream & out) const

{

out << "obstype  = \"" << obstype << "\"\n"
    << "dplat    = \"" << dplat   << "\"\n"
    << "isis     = \"" << isis    << "\"\n";

out << "jiter    = " << jiter  << '\n'
    << "nchanl   = " << nchanl << '\n'
    << "npred    = " << npred  << '\n'
    << "idate    = " << idate  << '\n'
    << "ireal    = " << ireal  << '\n'
    << "ipchan   = " << ipchan << '\n'
    << "iextra   = " << iextra << '\n'
    << "jextra   = " << jextra << '\n';

out << "idiag    = " << idiag    << '\n'
    << "angord   = " << angord   << '\n'
    << "iversion = " << iversion << '\n'
    << "inewpc   = " << inewpc   << '\n';


   //
   //  done
   //

out.flush();

return;

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

   mlog << Error << "\n\n  " << program_name << ": failed to read params\n\n";

   exit ( 1 );

}

// cout << "  read_params() -> n_read = " << n_read << '\n';

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

my_memcpy( &(params.idiag),    b, 4);
my_memcpy( &(params.angord),   b, 4);
my_memcpy( &(params.iversion), b, 4);
my_memcpy( &(params.inewpc),   b, 4);

if ( swap_endian )  {

   shuffle_4( &(params.jiter)  );
   shuffle_4( &(params.nchanl) );
   shuffle_4( &(params.npred)  );
   shuffle_4( &(params.idate)  );
   shuffle_4( &(params.ireal)  );
   shuffle_4( &(params.ipchan) );
   shuffle_4( &(params.iextra) );
   shuffle_4( &(params.jextra) );

   shuffle_4( &(params.idiag) );
   shuffle_4( &(params.angord) );
   shuffle_4( &(params.iversion) );
   shuffle_4( &(params.inewpc) );

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

   mlog << Error << "\n\n  " << program_name << ": failed to read channel info\n\n";

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
static int rec_num = 0;
long long size = 0;


size = peek_record_size(fd, rec_pad_length, swap_endian);

if ( size > buf_size )  {

   if ( buf )  { delete [] buf;  buf = 0; }

   buf = new unsigned char [size];

   buf_size = size;

}

n_read = read_fortran_binary(fd, buf, buf_size, rec_pad_length, swap_endian);

if ( n_read == 0 )  return ( false );

if ( n_read != bytes )  {

   mlog << Error << "  " << program_name << ": read_data() -> warning ... expected "
        << bytes << " bytes, got " << n_read << "\n";

   // exit ( 1 );

}


unsigned char * b = buf;

my_memcpy(diag,     b, 4*n_diag);
my_memcpy(diagchan, b, 4*n12);

if ( swap_endian )  {

   int j;

   for (j=0; j<n_diag; ++j)  shuffle_4(diag + j);

   for (j=0; j<n12; ++j)     shuffle_4(diagchan + j);

}

/*
int j, k;

for (j=0; j<n_diag; ++j)  {

   cout << rec_num << ' ' << "diag " << (j + 1) << " = " << diag[j] << '\n';

}


for (j=0; j<N1; ++j)  {

   k = fortran_two_to_one(N1,  j, channel);

   cout << rec_num << ' ' << "diagbuf[" << j << ", " << channel << "] = " << diagchan[k] << '\n';

}

cout << '\n';
*/

   //
   //  done
   //

++rec_num;

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

const int    obs_offset      = fortran_two_to_one(N1,  btemp_chan_index - 1, channel);
const int    omg_offset      = fortran_two_to_one(N1, omg_bc_chan_index - 1, channel);

const int    inv_obs_offset  = fortran_two_to_one(N1, inv_chan_index - 1, channel);
const int    surf_em_offset  = fortran_two_to_one(N1, surf_em_index  - 1, channel);

const double obs_value       = diagchan[obs_offset];
const double obs_minus_guess = diagchan[omg_offset];
const double guess           = obs_value - obs_minus_guess;

const double inv_obs_error   = diagchan[inv_obs_offset];
const double surf_em         = diagchan[surf_em_offset];


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




