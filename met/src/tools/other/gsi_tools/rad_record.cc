// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <vx_util.h>

#include "rad_record.h"
#include "rad_offsets.h"
#include "ftto.h"


////////////////////////////////////////////////////////////////////////


static void my_memcpy(void * to, unsigned char * & from, int n_bytes);


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct RadParams
   //


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


   //
   //  Code for struct ChannelParams
   //


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


   //
   //  Code for class RadRecord
   //


////////////////////////////////////////////////////////////////////////


RadRecord::RadRecord()

{

rad_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


RadRecord::~RadRecord()

{

}


////////////////////////////////////////////////////////////////////////


void RadRecord::rad_init_from_scratch()

{

rad_clear();

return;

}


////////////////////////////////////////////////////////////////////////


void RadRecord::rad_clear()

{

gsi_clear();

diag = 0;

diagchan = 0;

extra = 0;

Ndiag     = 0;
Ndiagchan = 0;
Nextra    = 0;

N1 = N2 = 0;

iextra = jextra = 0;

Date = 0;

return;

}


////////////////////////////////////////////////////////////////////////


double RadRecord::diag_data(int index) const

{

if ( (index < 0) || (index >= Ndiag) )  {

   mlog << Error << "\nRadRecord::diag_data(int) const -> "
        << "range check error ... value = "
        << index << " ... Ndiag = " << Ndiag << "\n\n";

   exit ( 1 );

}

return ( (double) (diag[index]) );

}


////////////////////////////////////////////////////////////////////////


double RadRecord::diagchan_data(int index, int channel) const

{

if ( (index < 0) || (index >= N1) )  {

   mlog << Error << "\nRadRecord::diagchan_data() -> "
        << "range check error on index ... " << index << "\n\n";

   exit ( 1 );

}

if ( (channel < 0) || (channel >= N2) )  {

   mlog << Error << "\nRadRecord::diagchan_data() -> "
        << "range check error on channel ... " << channel << "\n\n";

   exit ( 1 );

}

const int n = fortran_two_to_one(N1, index, channel);

return ( (double) (diagchan[n]) );

}


////////////////////////////////////////////////////////////////////////


double RadRecord::extra_data(int i, int j) const

{

if ( ! extra )  {

   mlog << Error << "\nRadRecord::extra_data(int, int) const -> "
        << "no \"extra\" data in this record!\n\n";

   exit ( 1 );

}

if ( (i < 0) || (i >= iextra) || (j < 0) || (j >= jextra) )  {

   mlog << Error << "\nRadRecord::extra_data(int, int) const -> "
        << "range check error\n\n";

   exit ( 1 );

}

const int n = fortran_two_to_one(iextra, i, j);

return ( (double) (extra[n]) );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class RadFile
   //


////////////////////////////////////////////////////////////////////////


RadFile::RadFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


RadFile::~RadFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


void RadFile::init_from_scratch()

{

Fd = -1;

C_params = 0;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void RadFile::close()

{

if ( Fd >= 0 )  { ::close(Fd);  Fd = -1; }

if ( C_params )  { delete [] C_params;  C_params = 0; }

SwapEndian = true;

RecPadSize = 4;

Filename.clear();

Nchannels = 0;

Date = 0;

Ndiag = 0;
Nrec  = 0;
Npair = 0;

N1 = N2 = 0;



return;

}


////////////////////////////////////////////////////////////////////////


void RadFile::read_rad_params()

{

long long n_read;
const int buf_size = 512;
unsigned char buf[buf_size];

n_read = read_fortran_binary(Fd, buf, buf_size, RecPadSize, SwapEndian);

if ( n_read < 72 )  {

   mlog << Error << "\n\n  RadFile::read_rad_params() -> failed to read params\n\n";

   exit ( 1 );

}


unsigned char * b = buf;


my_memcpy( &(R_params.isis),    b, 20);
my_memcpy( &(R_params.dplat),   b, 10);
my_memcpy( &(R_params.obstype), b, 10);

my_memcpy( &(R_params.jiter),   b, 4);
my_memcpy( &(R_params.nchanl),  b, 4);
my_memcpy( &(R_params.npred),   b, 4);
my_memcpy( &(R_params.idate),   b, 4);
my_memcpy( &(R_params.ireal),   b, 4);
my_memcpy( &(R_params.ipchan),  b, 4);
my_memcpy( &(R_params.iextra),  b, 4);
my_memcpy( &(R_params.jextra),  b, 4);

my_memcpy( &(R_params.idiag),    b, 4);
my_memcpy( &(R_params.angord),   b, 4);
my_memcpy( &(R_params.iversion), b, 4);
my_memcpy( &(R_params.inewpc),   b, 4);


if ( SwapEndian )  {

   shuffle_4( &(R_params.jiter)  );
   shuffle_4( &(R_params.nchanl) );
   shuffle_4( &(R_params.npred)  );
   shuffle_4( &(R_params.idate)  );
   shuffle_4( &(R_params.ireal)  );
   shuffle_4( &(R_params.ipchan) );
   shuffle_4( &(R_params.iextra) );
   shuffle_4( &(R_params.jextra) );

   shuffle_4( &(R_params.idiag) );
   shuffle_4( &(R_params.angord) );
   shuffle_4( &(R_params.iversion) );
   shuffle_4( &(R_params.inewpc) );

}

   //
   //  done
   //

return;

}


////////////////////////////////////////////////////////////////////////


void RadFile::read_channel(int n)

{

if ( (n < 0) || (n >= Nchannels) )  {

   mlog << Error << "\nRadFile::read_channel(int) -> "
        << "range check error\n\n";

   exit ( 1 );

}

long long n_read;
ChannelParams & cp = C_params[n];
const int buf_size = 512;
unsigned char buf[buf_size];

n_read = read_fortran_binary(Fd, buf, buf_size, RecPadSize, SwapEndian);

if ( n_read != 32 )  {

   mlog << Error << "\n\n  RadFile::read_channel(int) -> failed to read channel info\n\n";

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

if ( SwapEndian )  {

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


int RadFile::channel_val(const int n) const

{


if ( (n < 0) || (n >= Nchannels) )  {

   mlog << Error << "\nRadFile::channel_val(int) -> "
        << "range check error\n\n";

   exit ( 1 );

}

return ( C_params[n].nuchan );

}


////////////////////////////////////////////////////////////////////////


int RadFile::use_channel(const int n) const

{


if ( (n < 0) || (n >= Nchannels) )  {

   mlog << Error << "\nRadFile::use_channel(int) -> "
        << "range check error\n\n";

   exit ( 1 );

}

return ( C_params[n].iuse );

}


////////////////////////////////////////////////////////////////////////


bool RadFile::open(const char * path, bool _swap_endian, int _rec_pad_size)

{

close();

if ( (Fd = met_open(path, O_RDONLY)) < 0 )  {

   Fd = -1;

   return ( false );

}

Filename = path;

SwapEndian = _swap_endian;

RecPadSize = _rec_pad_size;

   //
   //  rad params
   //

int k;
int month, day, year, hour;

read_rad_params();


k = R_params.idate;   //  YYYYMMDDHH

month = (k/10000)%100;
day   = (k/100)%100;
year  = k/1000000;
hour  = k%100;

Date = mdyhms_to_unix(month, day, year, hour, 0, 0);

Nchannels = R_params.nchanl;

Ndiag = R_params.ireal;

N1 = R_params.idiag;
N2 = R_params.nchanl;

   //
   //  channel params
   //

C_params = new ChannelParams [Nchannels];

for (k=0; k<Nchannels; ++k)  {

   read_channel(k);

}

   //
   //  current file position
   //

off_t cur_pos = ::lseek(Fd, 0L, SEEK_CUR);

   //
   //  loop through the records to inventory the file.
   //  count the number of records and number of pairs.
   //

RadRecord r;
Nrec = 0;

while ( (*this) >> r )  {

   Nrec++;

}

Npair = Nrec * Nchannels;

   //
   //  rewind to the previous file position
   //

::lseek(Fd, cur_pos, SEEK_SET);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for misc functions
   //


////////////////////////////////////////////////////////////////////////


void my_memcpy(void * to, unsigned char * & from, int n_bytes)

{

memcpy(to, from, n_bytes);

from += n_bytes;


return;

}


////////////////////////////////////////////////////////////////////////


bool operator>>(RadFile & f, RadRecord & r)

{

int n_read, bytes;
const int n_diag = f.n_diag();
const int n12    = f.n12();
const int iextra = f.iextra();
const int jextra = f.jextra();


bytes  = (int) (n_diag + n12)*sizeof(float);

if ( f.has_extra() )  bytes += (int) (iextra*jextra)*sizeof(float);

long long s = peek_record_size(f.Fd, f.get_rec_pad_size(), f.get_swap_endian());

r.extend(s);

n_read = read_fortran_binary(f.Fd, r.Buf, r.Nalloc, f.get_rec_pad_size(), f.get_swap_endian());

if ( n_read == 0 )  return ( false );

if ( n_read != bytes )  {

   mlog << Error << " operator>>(RadFile &, RadRecord &) -> warning ... expected "
        << bytes << " bytes, got " << n_read << "\n";

   // exit ( 1 );

}

r.Ndiag     = n_diag;
r.Ndiagchan = n12;
r.Nextra    = iextra*jextra;

r.diag     = (float *) (r.Buf);
r.diagchan = (float *) (r.Buf + 4*n_diag);
r.extra    = 0;

if ( r.Nextra > 0 )  r.extra = (float *) (r.Buf + 4*n_diag + 4*n12);

r.N1 = f.N1;
r.N2 = f.N2;

r.Date = f.Date;

r.iextra = f.R_params.iextra;
r.jextra = f.R_params.jextra;

if ( f.get_swap_endian() )  {

   int j;
   const int n = r.Ndiag + r.Ndiagchan + r.Nextra;

   for (j=0; j<n; ++j)  shuffle_4(r.Buf + 4*j);

}


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////




