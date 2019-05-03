// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
#include <errno.h>
#include <string.h>

#include "read_fortran_binary.h"
#include "check_endian.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


static const int local_buf_size = 128;   //  plenty big


static unsigned char local_buf[local_buf_size];


typedef void (*ShuffleFunc)(void *);


////////////////////////////////////////////////////////////////////////


static long long get_rec_size(unsigned char * buf, const int rec_pad_length);

static bool try_fb(int fd, const off_t file_size, const unsigned int test_pad_len, const bool swap_endian);


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
unsigned long long rec_size_1, rec_size_2;
int bytes;

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

bytes = min<int>(rec_size_1, buf_size);   //  shouldn't be needed, given the above code

if ( (n_read = read(fd, buf, bytes)) != bytes )  {

   mlog << Error << "\n\n  read_fortran_binary() -> read error ... "
        << "tried to read " << bytes << " bytes, got "
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


long long read_fortran_binary_realloc(const int fd, void * & buf, int & buf_size,
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
int bytes;
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
   //  reallocate buffer, if needed
   //

if ( rec_size_1 > buf_size )  {

   unsigned char * B = (unsigned char *) buf;

   if ( B )  { delete [] B;  B = 0; }

   buf = new unsigned char [rec_size_1];

   buf_size = rec_size_1;

}

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

bytes = min<int>(rec_size_1, buf_size);

if ( (n_read = read(fd, buf, bytes)) != bytes )  {

   mlog << Error << "\n\n  read_fortran_binary() -> read error ... "
        << "tried to read " << bytes << " bytes, got "
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
unsigned char peek_buf[16];
long long size;


if ( rec_pad_length >= (int) sizeof(peek_buf) )  {

   mlog << Error << "\n\n  peek_record_size(int fd) -> local buffer too small\n\n";

   exit ( 1 );

}

n_read = read(fd, peek_buf, rec_pad_length);

if ( n_read == 0 )  return ( 0LL );

switch ( rec_pad_length )  {

   case 4:
      if ( swap_endian ) shuffle_4(peek_buf);
      break;

   case 8:
      if ( swap_endian ) shuffle_8(peek_buf);
      break;

   default:
      mlog << Error << "\n\n  peek_record_size() -> bad record pad length\n\n";
      exit ( 1 );
      break;

}   //  switch

size = get_rec_size(peek_buf, rec_pad_length);

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


bool get_fb_params(int fd, int & rec_pad_length, bool & swap_endian)

{

   //
   //  get file size
   //

struct stat s;
off_t file_size;

if ( fstat(fd, &s) < 0 )  {

   mlog << Error
        << "\n\n  get_fb_params() -> fstat failed! ... "
        << strerror(errno) << "\n\n";

   exit ( 1 );

}

file_size = s.st_size;

   //
   //  loop through possibilities
   //

int j, k;
bool status = false;

const bool swap            [2] = { true, false };

const unsigned int pad_len [2] = { 4, 8 };


for (j=0; j<2; ++j)  {

   for (k=0; k<2; ++k)  {

      if ( try_fb(fd, file_size, pad_len[j], swap[k]) )  {

         rec_pad_length = pad_len[j];

         swap_endian    = swap[k];

         status = true;

         break;

      }

   }

}


   //
   //  seek to beginning of file again
   //

if ( lseek(fd, 0, SEEK_SET) < 0 )  {

   mlog << Error
        << "\n\n  get_fb_params() -> lseek error ... "
        << strerror(errno) << "\n\n";

   exit ( 1 );

}

   //
   //  nope
   //

return ( status );

}


////////////////////////////////////////////////////////////////////////


bool try_fb(int fd, const off_t file_size, const unsigned int test_pad_len, const bool swap_endian)

{

unsigned long long rec_size_1, rec_size_2;
int n_read, bytes;
off_t offset;
unsigned char buf[8];
int * ibuf = (int *) buf;
long long * llbuf = (long long *) buf;
bool status = false;


   //
   //  seek to beginning of file
   //

offset = (off_t) 0;

if ( lseek(fd, offset, SEEK_SET) < 0 )  {

   mlog << Error
        << "\n\n  try_4() -> lseek error (1) ... "
        << strerror(errno) << "\n\n";

   exit ( 1 );

}

   //
   //  get 1st pad
   //

bytes = test_pad_len;

if ( (n_read = read(fd, buf, bytes)) < 0 )  {

   mlog << Error
        << "\n\n  try_fb() -> read error (1) ... "
        << strerror(errno) << "\n\n";

   exit ( 1 );

}

if ( test_pad_len == 4 )  {

   if ( swap_endian )  shuffle_4(buf);

   rec_size_1 = (unsigned long long) (*ibuf);

} else {

   if ( swap_endian )  shuffle_8(buf);

   rec_size_1 = *llbuf;

}


if ( (rec_size_1 + 2*test_pad_len) > ((unsigned long long) file_size) )  return ( false );

   //
   //  lseek to the end of this data record
   //

if ( lseek(fd, rec_size_1, SEEK_CUR) < 0 )  {

   mlog << Error
        << "\n\n  try_fb() -> lseek error (2) ... "
        << strerror(errno) << "\n\n";

   exit ( 1 );

}

   //
   //  get 2nd pad
   //

bytes = test_pad_len;

if ( (n_read = read(fd, buf, bytes)) < 0 )  {

   mlog << Error
        << "\n\n  try_fb() -> read error (2) ... "
        << strerror(errno) << "\n\n";

   exit ( 1 );

}

if ( test_pad_len == 4 )  {

   if ( swap_endian )  shuffle_4(buf);

   rec_size_2 = (unsigned long long) (*ibuf);

} else {

   if ( swap_endian )  shuffle_8(buf);

   rec_size_2 = *llbuf;

}

   //
   //  compare rec sizes
   //

if ( rec_size_1 == rec_size_2 )  status = true;


   //
   //  nope
   //

return ( status );

}


////////////////////////////////////////////////////////////////////////



