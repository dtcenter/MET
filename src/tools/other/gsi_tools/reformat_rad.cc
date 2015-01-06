

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


static ConcatString program_name;

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

static void my_memcpy(void * to, unsigned char * & from, int);

static void read_params(int fd, RadParams &);

static void read_channel(int fd, ChannelParams &);

static bool read_data(int fd, const int n_diag, const int N1, const int N2, float * diag, float * diagchan);


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
int in = -1;
int count;
ofstream out;
AsciiTable table;
StatHdrColumns columns;
RadParams params;
ChannelParams * channel = 0;
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

channel = new ChannelParams [params.nchanl];

for (j=0; j<(params.nchanl); ++j)  {

   read_channel(in, channel[j]);

}

   //
   //  read data
   //

const int n_diag = params.ireal;

diag = new float [n_diag];

const int N1 = params.ipchan + params.npred + 1;
const int N2 = params.nchanl;

diagchan = new float [N1*N2];

cout << "diag = " << n_diag << " floats\n";

cout << "diagchan = " << (N1*N2) << " floats\n";

count = 0;

while ( 1 )  {

   status = read_data(in, n_diag, N1, N2, diag, diagchan);

   if ( ! status )  break;

   ++count;

}   //  while


cout << "\n\n  Read " << count << " data records\n\n";


    //
    //  done
    //

// out << table;

close(in);  in = -1;

out.close();

if ( channel )  { delete [] channel;  channel = 0; }

if ( diag )  { delete [] diag;  diag = 0; }

if ( diagchan )  { delete [] diagchan;  diagchan = 0; }

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


void read_channel(int fd, ChannelParams & channel)

{

long long n_read;

n_read = read_fortran_binary(fd, buf, buf_size, rec_pad_length, swap_endian);

if ( n_read != 32 )  {

   cerr << "\n\n  " << program_name << ": failed to read channel info\n\n";

   exit ( 1 );

}

unsigned char * b = buf;

my_memcpy( &(channel.freq),   b, 4);
my_memcpy( &(channel.plo),    b, 4);
my_memcpy( &(channel.wave),   b, 4);
my_memcpy( &(channel.varch),  b, 4);
my_memcpy( &(channel.tlap),   b, 4);
my_memcpy( &(channel.iuse),   b, 4);
my_memcpy( &(channel.nuchan), b, 4);
my_memcpy( &(channel.ich),    b, 4);

if ( swap_endian )  {

   shuffle_4( &(channel.freq)   );
   shuffle_4( &(channel.plo)    );
   shuffle_4( &(channel.wave)   );
   shuffle_4( &(channel.varch)  );
   shuffle_4( &(channel.tlap)   );
   shuffle_4( &(channel.iuse)   );
   shuffle_4( &(channel.nuchan) );
   shuffle_4( &(channel.ich)    );

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

my_memcpy(diag,       b, 4*n_diag);
my_memcpy(diagchan,   b, 4*n12);

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




