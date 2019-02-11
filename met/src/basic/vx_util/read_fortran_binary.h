// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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
   //  returns number of bytes read, not counting record pads
   //
   //    reallocates the buffer if needed
   //
   //
   //    NOTE: "buf", if nonzero, MUST point to heap memory, 
   //           not stack or text segment memory
   //

extern long long read_fortran_binary_realloc(const int fd, 
                                             void * & buf, int & buf_size, 
                                             const int rec_pad_length, 
                                             const bool swap_endian);


////////////////////////////////////////////////////////////////////////


   //
   //  report the value in the next record length pad, without moving the read pointer
   //

extern long long peek_record_size(int fd, const int rec_pad_length, const bool swap_endian);


////////////////////////////////////////////////////////////////////////


   //
   //  try to figure out the record pad size and the endian-ness of a file
   //

extern bool get_fb_params(int fd, int & rec_pad_length, bool & swap_endian);


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_READ_FORTRAN_BINARY_H__  */


////////////////////////////////////////////////////////////////////////


