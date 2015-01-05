

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "read_fortran_binary.h"
#include "chech_endian.h"


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
      cerr << "\n\n  read_fortran_binary() -> bad record pad size ... "
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

if ( rec_size_1 > buf_size )  {

   cerr << "\n\n  read_fortran_binary() -> buffer too small ... "
        << "increase buffer size to at least " << rec_size_1 << " bytes!\n\n";

   exit ( 1 );

}

if ( (n_read = read(fd, buf, rec_size_1)) != rec_size_1 )  {

   cerr << "\n\n  read_fortran_binary() -> read error ... "
        << "tried to read " << rec_size_1 << " bytes, got "
        << n_read << "\n\n";

   exit ( 1 );

}

   //
   //  second record length pad
   //

n_read = ::read(fd, local_buf, rec_pad_length);

if ( n_read != rec_pad_length )  {

   cerr << "\n\n  read_fortran_binary() -> trouble reading second record pad\n\n";

   exit ( 1 );

}

if ( swap_endian )  shuffle(local_buf);

rec_size_2 = get_rec_size(local_buf, rec_pad_length);

if ( rec_size_2 != rec_size_1 )  {

   cerr << "\n\n  read_fortran_binary() -> first and second record pads don't match!\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( rec_size_1 );

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



