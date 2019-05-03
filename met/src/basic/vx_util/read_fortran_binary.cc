// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "read_fortran_binary.h"
#include "check_endian.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const int local_buf_size = 128;   //  plenty big


static unsigned char local_buf[local_buf_size];


typedef void (*ShuffleFunc)(void *);


////////////////////////////////////////////////////////////////////////


static long long get_rec_size(unsigned char * buf, const int rec_pad_length);


////////////////////////////////////////////////////////////////////////


long long read_fortran_binary(const int fd, void * buf, const int buf_size, 
                              const int rec_pad_length, 
                              const bool swap_endian)

{

ShuffleFunc shuffle = 0;

   //
   //  we don't "shuffle" the data ... only the record pads
   //

switch ( rec_pad_length )  {

   case 4:  shuffle = shuffle_4;  break;
   case 8:  shuffle = shuffle_8;  break;

   default:
      mlog << Error << "\n\n  read_fortran_binary() -> bad record pad size ... "
           << rec_pad_length << "\n\n";
      exit ( 1 );
      break;

}

int n_read;
long long rec_size_1, rec_size_2;

   //
   //  first record length pad
   //

n_read = ::read(fd, local_buf, rec_pad_length);

if ( n_read == 0 )  return ( 0LL );

if ( n_read < 0 )  return ( (long long) (-1) );

if ( swap_endian )  shuffle(local_buf);

rec_size_1 = get_rec_size(local_buf, rec_pad_length);

   //
   //  data
   //

if ( rec_size_1 < 0 || rec_size_1 > buf_size )  {

   mlog << Error << "\n\n  read_fortran_binary() -> buffer too small ... "
        << "increase buffer size to at least " << rec_size_1 << " bytes!\n"
        << "  Try using the -swap option to switch the endianness of the "
        << "input binary files.\n\n";

   exit ( 1 );

}

if ( (n_read = read(fd, buf, rec_size_1)) != rec_size_1 )  {

   mlog << Error << "\n\n  read_fortran_binary() -> read error ... "
        << "tried to read " << rec_size_1 << " bytes, got "
        << n_read << "\n\n";

   exit ( 1 );

}

   //
   //  second record length pad
   //

n_read = ::read(fd, local_buf, rec_pad_length);

if ( n_read != rec_pad_length )  {

   mlog << Error << "\n\n  read_fortran_binary() -> trouble reading second record pad\n\n";

   exit ( 1 );

}

if ( swap_endian )  shuffle(local_buf);

rec_size_2 = get_rec_size(local_buf, rec_pad_length);

if ( rec_size_2 != rec_size_1 )  {

   mlog << Error << "\n\n  read_fortran_binary() -> first and second record pads don't match!\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( rec_size_1 );

}


////////////////////////////////////////////////////////////////////////


long long peek_record_size(int fd, const int rec_pad_length, const bool swap_endian)

{

int n_read;
unsigned char local_buf[16];
long long size;


if ( rec_pad_length >= (int) sizeof(local_buf) )  {

   mlog << Error << "\n\n  peek_record_size(int fd) -> local buffer too small\n\n";

   exit ( 1 );

}

n_read = read(fd, local_buf, rec_pad_length);

if ( n_read == 0 )  return ( 0LL );

switch ( rec_pad_length )  {

   case 4:            
      if ( swap_endian ) shuffle_4(local_buf);
      break;

   case 8:
      if ( swap_endian ) shuffle_8(local_buf);
      break;

   default:
      mlog << Error << "\n\n  peek_record_size() -> bad record pad length\n\n";
      exit ( 1 );
      break;

}   //  switch

size = get_rec_size(local_buf, rec_pad_length);

   //
   //  put the read pointer back
   //

if ( lseek(fd, -rec_pad_length, SEEK_CUR) < 0 )  {

   mlog << Error << "\n\n  peek_record_size() -> lseek failed!\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( size );

}


////////////////////////////////////////////////////////////////////////


long long get_rec_size(unsigned char * buf, const int rec_pad_length)

{

long long s;

   //
   //  we know that rec_pad_length is either 4 or 8
   //

if ( rec_pad_length == 4 )  {

   int * k = (int *) buf;

   s = (long long) (*k);

} else {   //  8

   long long * a = (long long *) buf;

   s = *a;

}


return ( s );

}


////////////////////////////////////////////////////////////////////////



