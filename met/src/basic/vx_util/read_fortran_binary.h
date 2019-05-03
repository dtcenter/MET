// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __VX_READ_FORTRAN_BINARY_H__
#define  __VX_READ_FORTRAN_BINARY_H__


////////////////////////////////////////////////////////////////////////

   //
   //  returns number of bytes read, not counting record pads
   //

extern long long read_fortran_binary(const int fd, void * buf, const int buf_size, 
                                     const int rec_pad_length, 
                                     const bool swap_endian);


////////////////////////////////////////////////////////////////////////


   //
   //  report the value in the next record length pad, without moving the read pointer
   //

extern long long peek_record_size(int fd, const int rec_pad_length, const bool swap_endian);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_READ_FORTRAN_BINARY_H__  */


////////////////////////////////////////////////////////////////////////


