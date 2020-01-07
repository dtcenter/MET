// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



//////////////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include "vx_log.h"
#include "copy_bytes.h"
#include "do_blocking.h"


//////////////////////////////////////////////////////////////////////////////////


static void write_pad(int fd, unsigned int value, PadSize);

static int find_magic_cookie(int fd);

static int get_record_size(int fd);


//////////////////////////////////////////////////////////////////////////////////


void do_blocking(int in, int out, PadSize padsize)

{

unsigned int nullpad;
unsigned int rec_size;
int n_written;
unsigned char buf[8];


while ( find_magic_cookie(in) >= 0 )  {

   rec_size = get_record_size(in);

   nullpad = 8 - rec_size%8;

      //
      //  first pad
      //

   write_pad(out, rec_size + nullpad, padsize);

      //
      //  copy the record
      //

   copy_bytes(in, out, rec_size);

      //
      //  null-pad the record to a multiple of 8 bytes (because cwordsh does this)
      //

   memset(buf, 0, sizeof(buf));

   if ( (n_written = write(out, buf, nullpad)) != nullpad )  {

      mlog << Error << "\ndo_blocking() -> "
           << "write error in null-padding, n_written = " << n_written << "\n\n";

      exit ( 1 );

   }

      //
      //  second pad
      //

   write_pad(out, rec_size + nullpad, padsize);

}   //  while

   //
   //  done
   //

return;

}


//////////////////////////////////////////////////////////////////////////////////


void write_pad(int fd, unsigned int value, PadSize padsize)

{

int n_written, bytes;
unsigned int I;
unsigned long long L;
unsigned char * b = (unsigned char *) 0;


switch ( padsize )  {

   case padsize_4:
      bytes = 4;
      b = (unsigned char *) (&I);
      I = (unsigned int) value;
      break;

   case padsize_8:
      bytes = 8;
      b = (unsigned char *) (&L);
      L = (unsigned long long) value;
      break;

   default:
      mlog << Error << "\nwrite_pad() -> "
           << "bad pad size\n\n";
      exit ( 1 );
      break;

}   //  switch

if ( (n_written = write(fd, b, bytes)) != bytes )  {

   mlog << Error << "\nwrite_pad() -> "
        << "write error, n_written = " << n_written << "\n\n";

   exit ( 1 );

}

return;

}


//////////////////////////////////////////////////////////////////////////////////


int find_magic_cookie(int fd)

{

int j, n_read;
int pos = 0;
const int temp_buf_size = 128;
char tempbuf[temp_buf_size];


while ( (n_read = read(fd, tempbuf, sizeof(tempbuf))) > 0 )  {

   pos = lseek(fd, 0L, SEEK_CUR) - n_read;

   for (j=0; j<=(n_read - 4); ++j)  {

      if ( (tempbuf[j] == 'B') && (tempbuf[j + 1] == 'U') && (tempbuf[j + 2] == 'F') && (tempbuf[j + 3] == 'R') )  {

         pos += j;

         // lseek(fd, pos + 2, SEEK_SET);
            lseek(fd, pos, SEEK_SET);

         return ( pos );

      }

   }   //  for j

   if ( n_read < 10 )  return ( -1 );

   pos += n_read - 5;

   lseek(fd, pos, SEEK_SET);

}   //  while

if ( n_read == 0 )  return ( -1 );

if ( n_read < 0 )  {

   mlog << Error << "\nfind_magic_cookie() -> "
        << "trouble reading file\n\n";

   exit ( 1 );

}

return ( pos );

}


//////////////////////////////////////////////////////////////////////////////////


int get_record_size(int fd)

{

int pos;
int rec_size;
int n_read, bytes;
unsigned char tempbuf[8];
int edition_number;


   //
   //  remember the current file position
   //

if ( (pos = lseek(fd, 0L, SEEK_CUR)) < 0 )  {

   mlog << Error << "\nget_record_size() -> "
        << "lseek error (1)\n\n";

   exit ( 1 );

}

   //
   //  Read the first 8 bytes of Section 0 (Indicator section)
   //

bytes = (int) sizeof(tempbuf);

if ( (n_read = read(fd, tempbuf, bytes)) != bytes )  {

   mlog << Error << "\nget_record_size() -> "
        << "read error, n_read = " << n_read << "\n\n";

   exit ( 1 );

}

   //
   //  check edition number
   //

edition_number = (int) (tempbuf[7]);

if ( edition_number < 2 )  {

   mlog << Error << "\nget_record_size() -> "
        << "This program works only on prepbufr records\n"
        << "with edition number >= 2.  The edition number\n"
        << "of this record is " << edition_number
        << ".\n\n";

   exit ( 1 );

}

   //
   //  get record size
   //
   //   this arithmetic should work on either big-endian
   //   or little-endian machines
   //

rec_size = 0;

rec_size = 256*rec_size + tempbuf[4];
rec_size = 256*rec_size + tempbuf[5];
rec_size = 256*rec_size + tempbuf[6];


   //
   //  move back to the original position
   //

if ( lseek(fd, pos, SEEK_SET) < 0 )  {

   mlog << Error << "\nget_record_size() -> "
        << "lseek error (2)\n\n";

   exit ( 1 );

}

   //
   //  done
   //

return ( rec_size );

}


//////////////////////////////////////////////////////////////////////////////////



