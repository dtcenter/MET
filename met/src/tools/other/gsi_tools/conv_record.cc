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

#include "conv_record.h"
#include "conv_offsets.h"
#include "ftto.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ConvRecord
   //


////////////////////////////////////////////////////////////////////////


ConvRecord::ConvRecord()

{

conv_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ConvRecord::~ConvRecord()

{

}


////////////////////////////////////////////////////////////////////////


void ConvRecord::conv_init_from_scratch()

{

conv_clear();


return;

}


////////////////////////////////////////////////////////////////////////


void ConvRecord::conv_clear()

{

gsi_clear();

nchar = nreal = ii = mtype = 0;

n_rdiag = 0;

cdiag_bytes = 0;

rdiag_bytes = 0;

date = 0;

cdiag = 0;

rdiag = 0;


return;

}


////////////////////////////////////////////////////////////////////////


double ConvRecord::rdiag_get_2d(int data_index, int station) const //  zero-based

{

const int n = fortran_two_to_one(nreal, data_index, station);

double value = (double) (rdiag[n]);


return ( value );

}


////////////////////////////////////////////////////////////////////////


ConcatString ConvRecord::station_name(int index) const  //  zero-based

{

if ( (index < 0) || (index >= ii) )  {

   mlog << Error << "\nConvRecord::station_name() const -> "
        << "range check error\n\n";

   exit ( 1 );

}

char localbuf[9];   //  station names are at most 8 characters long

memcpy(localbuf, cdiag + 8*index, 8);

localbuf[8] = (char) 0;

int j;

for (j=7; j>=0; --j)  {

   if ( localbuf[j] == ' ' )  localbuf[j] = (char) 0;

   else break;

}


return ( ConcatString(localbuf) );

}


////////////////////////////////////////////////////////////////////////


ConcatString ConvRecord::date_string() const

{

int month, day, year, hour, minute, second;
char junk[256];

unix_to_mdyhms(date, month, day, year, hour, minute, second);

snprintf(junk, sizeof(junk), "%04d%02d%02d_%02d0000", year, month, day, hour);

return ( ConcatString(junk) );

}


////////////////////////////////////////////////////////////////////////


double ConvRecord::rdiag_get_guess(int station) const

{

double obs, obs_minus_guess;

obs             = rdiag_get_2d(conv_obs_data_index - 1, station);
obs_minus_guess = rdiag_get_2d(conv_omg_index      - 1, station);

return ( obs - obs_minus_guess );

}


////////////////////////////////////////////////////////////////////////


double ConvRecord::rdiag_get_guess_v(int station) const

{

double obs, obs_minus_guess;

obs             = rdiag_get_2d(conv_obs_v_data_index - 1, station);
obs_minus_guess = rdiag_get_2d(conv_omg_v_index      - 1, station);

return ( obs - obs_minus_guess );

}


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class ConvFile
   //


////////////////////////////////////////////////////////////////////////


ConvFile::ConvFile()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


ConvFile::~ConvFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


void ConvFile::init_from_scratch()

{

Fd = -1;

close();

return;

}


////////////////////////////////////////////////////////////////////////


void ConvFile::close()

{

if ( Fd >= 0 )  { ::close(Fd);  Fd = -1; }

Date = 0;

SwapEndian = true;

RecPadSize = 4;

Nrec  = 0;
Npair = 0;

Filename.clear();

return;

}


////////////////////////////////////////////////////////////////////////


bool ConvFile::open(const char * path, bool _swap_endian, int _rec_pad_size)

{

close();

if ( (Fd = met_open(path, O_RDONLY)) < 0 )  {

   Fd = -1;

   return ( false );

}

Filename = path;

SwapEndian = _swap_endian;

RecPadSize = _rec_pad_size;

int idate;
int year, month, day, hour;

   //
   //  read the date
   //

long long s = read_fortran_binary(Fd, &idate, (int) sizeof(idate), RecPadSize, SwapEndian);

if ( s <= 0 )  {

   mlog << Warning << "\nConvFile::open() -> "
        << "unable to read date from input file: " << Filename << "\n\n";

   return ( false );

}

if ( SwapEndian )  shuffle_4(&idate);

year  = idate/1000000;
month = (idate/10000)%100;
day   = (idate/100)%100;
hour  = idate%100;

Date = mdyhms_to_unix(month, day, year, hour, 0, 0);

   //
   //  current file position
   //

off_t cur_pos = ::lseek(Fd, 0L, SEEK_CUR);

   //
   //  loop through the records to inventory the file.
   //  count the number of records and number of pairs.
   //  two pairs for "uv" variable and 1 for all others.
   //

ConvRecord r;
Nrec = Npair = 0;

while ( (*this) >> r )  {
   Nrec++;
   Npair += ((str_trim(r.variable) == "uv") ? 2 : 1) * r.ii;
}

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


bool operator>>(ConvFile & f, ConvRecord & r)

{

r.date = f.Date;

   //
   //  read the cdiag data
   //

r.extend(512);

long long s = read_fortran_binary(f.Fd, r.Buf, 23, f.RecPadSize, f.SwapEndian);

if ( s == 0 )  return ( false );

if ( s != 19 && s != 23 )  {

   mlog << Error << "\noperator>>(ConvFile &, ConvRecord &) -> "
        << "trouble reading cdiag data\n\n";

   exit ( 1 );

}

 r.variable = (string)(char*)r.Buf;

//r.variable[3] = (char) 0;

memcpy(&(r.nchar), r.Buf +  3, 4);
memcpy(&(r.nreal), r.Buf +  7, 4);
memcpy(&(r.ii),    r.Buf + 11, 4);
memcpy(&(r.mtype), r.Buf + 15, 4);

if ( f.SwapEndian )  {

   shuffle_4(&(r.nchar));
   shuffle_4(&(r.nreal));
   shuffle_4(&(r.ii));
   shuffle_4(&(r.mtype));

}

r.n_rdiag = (r.nreal)*(r.ii);

r.cdiag_bytes = 8*(r.ii);
r.rdiag_bytes = 4*(r.n_rdiag);


   //
   //  read rdiag data
   //

s = peek_record_size(f.Fd, f.RecPadSize, f.SwapEndian);

r.extend(s);

s = read_fortran_binary(f.Fd, r.Buf, r.Nalloc, f.RecPadSize, f.SwapEndian);

r.cdiag = (char *)  (r.Buf);
r.rdiag = (float *) (r.Buf + r.cdiag_bytes);

if ( f.SwapEndian )  {

   int j;

   for (j=0; j<(r.n_rdiag); ++j)  shuffle_4(r.rdiag + j);

}


   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////




