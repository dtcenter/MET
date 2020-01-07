// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MET_BUFFER_H__
#define  __MET_BUFFER_H__


////////////////////////////////////////////////////////////////////////


#include "stdlib.h"


////////////////////////////////////////////////////////////////////////


typedef long long bigint;


////////////////////////////////////////////////////////////////////////

   //
   //  Looks to the user like an array of unsigned chars,
   //   but is reallocatable
   //

class MetBuffer {

   protected:

      void mb_init_from_scratch();

      void mb_assign(const MetBuffer &);

      unsigned char * Buf;   //  allocated

      bigint Nbytes;   //  from last read
      bigint Nalloc;   //  from last read

         //  these are for reading fortran binary records

      int  RecPadSize;   //  default: 4
      bool SwapEndian;   //  default: false

   public:

      MetBuffer();
      MetBuffer(int _initial_size_);
     ~MetBuffer();
      MetBuffer(const MetBuffer &);
      MetBuffer & operator=(const MetBuffer &);

      void mb_clear();

         //
         //  set stuff
         //

      void set_rec_pad_size(int);

      void set_swap_endian(bool = true);

         //
         //  get stuff
         //

      unsigned char * operator()() const;

      bigint n_alloc () const;   //  in bytes
      bigint n_bytes () const;   //  in bytes

         //
         //  do stuff
         //

      void extend(bigint _bytes_);

      int read(const int _fd_, const bigint _bytes_);   //  Read from a file descriptor.
                                                        //  Returns # of bytes actually read, or
                                                        //  0 for end-of-file, or -1 for error
                                                        //  just like the standard library read function.

      bool read_fortran(const int _fd_);   //  read a fortran record from a file descriptor

};


////////////////////////////////////////////////////////////////////////


inline unsigned char * MetBuffer::operator()() const { return ( Buf ); }

inline bigint MetBuffer::n_bytes() const { return ( Nbytes ); }
inline bigint MetBuffer::n_alloc() const { return ( Nalloc ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_BUFFER_H__  */


////////////////////////////////////////////////////////////////////////


